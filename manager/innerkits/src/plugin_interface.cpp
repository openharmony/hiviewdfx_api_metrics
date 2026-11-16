/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 *
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "plugin_interface.h"
#include "ihistogram_plugin.h"
#include "plugin_manager.h"
#include "log_wrapper.h"

#include <chrono>
#include <thread>
#include <atomic>
#include <limits>
#include <memory>
#include <mutex>
#include <utility>

#ifndef HISTOGRAM_LIKELY
#define HISTOGRAM_LIKELY(x) __builtin_expect(!!(x), 1)
#endif

#ifndef HISTOGRAM_UNLIKELY
#define HISTOGRAM_UNLIKELY(x) __builtin_expect(!!(x), 0)
#endif

namespace OHOS {
namespace histogram {
namespace {

/*
 * Common numeric constraints.
 *
 * Histogram sample values are restricted to non-negative int32_t values.
 * The upper bound is exclusive to avoid overflow in downstream bucket
 * calculations and index derivation.
 */
constexpr int32_t SAMPLE_MIN = 0;
constexpr int32_t SAMPLE_UPPER_EXCLUSIVE =
    std::numeric_limits<int32_t>::max();

/*
 * Enumeration histogram constraints.
 *
 * boundary must be in range (0, 500].
 */
constexpr int32_t ENUM_BOUNDARY_MAX = 500;

/*
 * Count histogram bucket count constraints.
 *
 * Valid range: [2, 100].
 */
constexpr size_t MIN_BUCKET_COUNT = 2;
constexpr size_t MAX_BUCKET_COUNT = 100;

/*
 * Cached plugin instance.
 *
 * After the first successful lazy load, subsequent calls use the cached
 * pointer to avoid repeated manager lookup on hot paths.
 */
static std::atomic<IHistogramPlugin *> g_plugin { nullptr };
static std::atomic<bool> g_pluginLoading { false };

/*
 * Validates common sample values.
 *
 * Valid range: [0, int32_t::max()).
 */
inline bool IsValidSample(int32_t sample)
{
    return sample >= SAMPLE_MIN &&
           sample < SAMPLE_UPPER_EXCLUSIVE;
}

/*
 * Validates boolean histogram samples.
 *
 * Supported values:
 *   0 - false
 *   1 - true
 */
inline bool IsValidBooleanSample(int32_t sample)
{
    return sample == 0 || sample == 1;
}

/*
 * Validates enumeration histogram parameters.
 *
 * boundary range:
 *   (0, 500]
 *
 * sample range:
 *   [0, boundary]
 */
inline bool IsValidEnumSample(int32_t sample, int32_t boundary)
{
    return boundary > 0 &&
           boundary <= ENUM_BOUNDARY_MAX &&
           sample >= 0 &&
           sample <= boundary;
}

/*
 * Validates bucket count for count histograms.
 *
 * Valid range:
 *   [2, 100]
 */
inline bool IsValidBucketCount(size_t bucketCount)
{
    return bucketCount >= MIN_BUCKET_COUNT &&
           bucketCount <= MAX_BUCKET_COUNT;
}

/*
 * Validates count histogram min/max range.
 *
 * min range:
 *   [0, int32_t::max())
 *
 * max range:
 *   (min, int32_t::max())
 */
inline bool IsValidCountRange(int32_t min, int32_t max)
{
    return min >= SAMPLE_MIN &&
           min < SAMPLE_UPPER_EXCLUSIVE &&
           max > min &&
           max < SAMPLE_UPPER_EXCLUSIVE;
}

/*
 * Returns true when metric name is invalid.
 *
 * Empty names are rejected to prevent anonymous histogram registration.
 */
inline bool IsInvalidName(const std::string &name)
{
    return name.empty();
}

/*
 * Loads plugin instance on cache miss.
 *
 * This path is expected to run only during first use or initialization
 * recovery scenarios.
 */
inline IHistogramPlugin *LoadPluginSlowPath()
{
    PluginManager &manager = PluginManager::GetInstance();
    IHistogramPlugin *plugin = manager.GetPlugin();
    if (plugin != nullptr) {
        g_plugin.store(plugin, std::memory_order_release);
        return plugin;
    }
    if (HISTOGRAM_UNLIKELY(!manager.LazyLoadPlugin())) {
        return nullptr;
    }
    plugin = manager.GetPlugin();
    if (plugin != nullptr) {
        g_plugin.store(plugin, std::memory_order_release);
    }
    return plugin;
}

/*
 * Owns the asynchronous plugin-loading thread.
 *
 * The thread used to be detached, which meant it could still be running
 * dlopen()/dlsym()/dlclose() while the process was exiting. During exit,
 * __cxa_finalize() destroys the module's statics, so the thread would race
 * against teardown of the very module it was executing in.
 *
 * Instead of detaching, the handle is kept here and joined before process
 * exit. This guard is a function-local static created on first use, i.e.
 * AFTER the namespace-scope statics of this module; C++ destroys
 * function-local statics in reverse order of construction, so this guard is
 * destroyed BEFORE them and the thread is joined while the module is intact.
 *
 * The loading path stays fully asynchronous: callers still return immediately
 * and the hot path (plugin already loaded) never creates a thread.
 */
struct LoadThreadGuard {
    std::mutex mtx;
    std::thread worker;
    // Shared with the worker so that a detached worker never touches this
    // guard after it has been destroyed.
    std::shared_ptr<std::atomic<bool>> finished { std::make_shared<std::atomic<bool>>(true) };

    ~LoadThreadGuard()
    {
        std::thread pending;
        std::shared_ptr<std::atomic<bool>> state;
        {
            std::lock_guard<std::mutex> lock(mtx);
            pending = std::move(worker);
            state = finished;
        }
        if (!pending.joinable()) {
            return;
        }
        // Bounded wait: dlopen() is normally a few milliseconds, but a slow or
        // stalled filesystem must not hold process exit hostage. Give the
        // loading thread a short grace period, then detach and let the process
        // finish exiting.
        constexpr int kMaxWaitMs = 500;
        constexpr int kPollMs = 10;
        for (int waited = 0; waited < kMaxWaitMs; waited += kPollMs) {
            if (state->load(std::memory_order_acquire)) {
                pending.join();
                return;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(kPollMs));
        }
        HISTOGRAM_ERROR_LOG("plugin_manager: async load thread did not finish in %{public}d ms, "
            "detaching to avoid blocking process exit", kMaxWaitMs);
        pending.detach();
    }
};

inline LoadThreadGuard &GetLoadThreadGuard()
{
    static LoadThreadGuard guard;
    return guard;
}

/*
 * Loads plugin instance Async.
 *
 * This async path is to dlopen so
 */
template <typename Task>
inline bool StartLoadPluginAsync(Task &&loadTask)
{
    bool expected = false;
    if (!g_pluginLoading.compare_exchange_strong(
        expected,
        true,
        std::memory_order_acq_rel,
        std::memory_order_acquire)) {
        return false;
    }

    // Force PluginManager construction before the guard so that the guard,
    // constructed later, is destroyed earlier: the loading thread is joined
    // before ~PluginManager() calls UnloadAllPlugins()/dlclose().
    (void)PluginManager::GetInstance();

    LoadThreadGuard &guard = GetLoadThreadGuard();

    std::lock_guard<std::mutex> lock(guard.mtx);
    if (guard.worker.joinable()) {
        // A previous load attempt failed and is being retried; reclaim it so
        // that failed loads cannot accumulate threads. It is expected to have
        // finished already (g_pluginLoading gates concurrent attempts).
        guard.worker.join();
    }

    // Reset only after the previous attempt is reclaimed, so the exit-time
    // bounded wait cannot observe a stale "finished" flag.
    guard.finished->store(false, std::memory_order_release);

    // The worker holds its own reference to the completion flag, so it stays
    // valid even if this guard is destroyed first (detach-after-timeout path).
    std::shared_ptr<std::atomic<bool>> finished = guard.finished;
    std::thread worker([asyncTask = std::forward<Task>(loadTask), finished]() mutable {
        // A local class cannot capture the enclosing lambda's captures, so it
        // holds its own reference to the completion flag as a member.
        struct LoadingGuard {
            std::shared_ptr<std::atomic<bool>> finished;
            ~LoadingGuard()
            {
                g_pluginLoading.store(false, std::memory_order_release);
                finished->store(true, std::memory_order_release);
            }
        } loadingGuard { finished };

        IHistogramPlugin *plugin = LoadPluginSlowPath();
        if (plugin != nullptr) {
            asyncTask(plugin);
        }
    });

    guard.worker = std::move(worker);

    return true;
}

/*
 * Returns plugin instance using fast path cache.
 *
 * Hot path cost:
 *   one atomic load and one null check.
 */
inline IHistogramPlugin *GetPluginFast()
{
    IHistogramPlugin *plugin = g_plugin.load(std::memory_order_acquire);

    if (HISTOGRAM_LIKELY(plugin != nullptr)) {
        return plugin;
    }

    return nullptr;
}

}  // namespace

/*
 * Records a boolean histogram sample.
 *
 * sample values:
 *   0 or 1
 */
int32_t PluginInterface::AddBooleanSample(const std::string &name, int32_t sample)
{
    if (HISTOGRAM_UNLIKELY(IsInvalidName(name))) {
        return -1;
    }

    if (HISTOGRAM_UNLIKELY(!IsValidBooleanSample(sample))) {
        return -1;
    }

    IHistogramPlugin *plugin = GetPluginFast();
    if (HISTOGRAM_UNLIKELY(plugin == nullptr)) {
        if (StartLoadPluginAsync(
            [name, sample](IHistogramPlugin *plugin) { plugin->AddBooleanSample(name.c_str(), sample); })) {
            return 0;
        }
        return -1;
    }

    return plugin->AddBooleanSample(name.c_str(), sample);
}

/*
 * Records an enumeration histogram sample.
 *
 * boundary:
 *   (0, 500]
 *
 * sample:
 *   [0, boundary]
 */
int32_t PluginInterface::AddEnumSample(const std::string &name, int32_t sample, int32_t boundary)
{
    if (HISTOGRAM_UNLIKELY(IsInvalidName(name))) {
        return -1;
    }

    if (HISTOGRAM_UNLIKELY(!IsValidEnumSample(sample, boundary))) {
        return -1;
    }

    IHistogramPlugin *plugin = GetPluginFast();
    if (HISTOGRAM_UNLIKELY(plugin == nullptr)) {
        if (StartLoadPluginAsync([name, sample, boundary](IHistogramPlugin *plugin) {
                plugin->AddEnumSample(name.c_str(), sample, boundary);
            })) {
            return 0;
        }
        return -1;
    }

    return plugin->AddEnumSample(name.c_str(), sample, boundary);
}

/*
 * Records a count histogram sample.
 *
 * sample:
 *   [0, int32_t::max())
 *
 * min:
 *   [0, int32_t::max())
 *
 * max:
 *   (min, int32_t::max())
 *
 * bucketCount:
 *   [2, 100]
 */
int32_t PluginInterface::AddCountSample(const std::string &name, int32_t sample, int32_t min, int32_t max,
                                        size_t bucketCount)
{
    if (HISTOGRAM_UNLIKELY(IsInvalidName(name))) {
        return -1;
    }

    if (HISTOGRAM_UNLIKELY(!IsValidSample(sample))) {
        return -1;
    }

    if (HISTOGRAM_UNLIKELY(!IsValidCountRange(min, max))) {
        return -1;
    }

    if (HISTOGRAM_UNLIKELY(!IsValidBucketCount(bucketCount))) {
        return -1;
    }

    IHistogramPlugin *plugin = GetPluginFast();
    if (HISTOGRAM_UNLIKELY(plugin == nullptr)) {
        if (StartLoadPluginAsync([name, sample, min, max, bucketCount](IHistogramPlugin *plugin) {
                plugin->AddCountSample(name.c_str(), sample, min, max, bucketCount);
            })) {
            return 0;
        }
        return -1;
    }

    return plugin->AddCountSample(name.c_str(), sample, min, max, bucketCount);
}

/*
 * Records a time histogram sample.
 *
 * sample:
 *   [0, int32_t::max())
 */
int32_t PluginInterface::AddTimeSample(const std::string &name, int32_t sample)
{
    if (HISTOGRAM_UNLIKELY(IsInvalidName(name))) {
        return -1;
    }

    if (HISTOGRAM_UNLIKELY(!IsValidSample(sample))) {
        return -1;
    }

    IHistogramPlugin *plugin = GetPluginFast();
    if (HISTOGRAM_UNLIKELY(plugin == nullptr)) {
        if (StartLoadPluginAsync(
            [name, sample](IHistogramPlugin *plugin) { plugin->AddTimeSample(name.c_str(), sample); })) {
            return 0;
        }
        return -1;
    }

    return plugin->AddTimeSample(name.c_str(), sample);
}

/*
 * Records a percentage histogram sample.
 *
 * sample:
 *   [0, 100]
 */
int32_t PluginInterface::AddPercentageSample(const std::string &name, int32_t sample)
{
    if (HISTOGRAM_UNLIKELY(IsInvalidName(name))) {
        return -1;
    }

    if (HISTOGRAM_UNLIKELY(sample < 0 || sample > 100)) {
        return -1;
    }

    IHistogramPlugin *plugin = GetPluginFast();
    if (HISTOGRAM_UNLIKELY(plugin == nullptr)) {
        if (StartLoadPluginAsync(
            [name, sample](IHistogramPlugin *plugin) {
                plugin->AddPercentageSample(name.c_str(), sample);
            })) {
            return 0;
        }
        return -1;
    }

    return plugin->AddPercentageSample(name.c_str(), sample);
}

}  // namespace histogram
}  // namespace OHOS
