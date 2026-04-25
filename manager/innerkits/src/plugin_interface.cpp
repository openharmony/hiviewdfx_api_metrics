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

#include <atomic>
#include <limits>

#ifndef HISTOGRAM_LIKELY
#define HISTOGRAM_LIKELY(x) __builtin_expect(!!(x), 1)
#endif

#ifndef HISTOGRAM_UNLIKELY
#define HISTOGRAM_UNLIKELY(x) __builtin_expect(!!(x), 0)
#endif

namespace OHOS {
namespace histogram {
namespace {

constexpr int32_t SAMPLE_MIN = 0;
constexpr int32_t SAMPLE_MAX = std::numeric_limits<int32_t>::max();
constexpr size_t MIN_BUCKET_COUNT = 2;

static std::atomic<IHistogramPlugin *> g_plugin { nullptr };

inline bool IsValidSample(int32_t sample)
{
    return sample >= SAMPLE_MIN && sample < SAMPLE_MAX;
}

inline bool IsValidBooleanSample(int32_t sample)
{
    return sample == 0 || sample == 1;
}

inline bool IsValidEnumSample(int32_t sample, int32_t boundary)
{
    return boundary > 0 && sample >= 0 && sample < boundary;
}

inline bool IsValidBucketCount(size_t bucketCount)
{
    return bucketCount >= MIN_BUCKET_COUNT;
}

inline bool IsValidRange(int32_t min, int32_t max)
{
    return min < max;
}

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

inline IHistogramPlugin *GetPluginFast()
{
    IHistogramPlugin *plugin = g_plugin.load(std::memory_order_acquire);

    if (HISTOGRAM_LIKELY(plugin != nullptr)) {
        return plugin;
    }

    return LoadPluginSlowPath();
}

inline bool IsNullName(const std::string &name)
{
    return name.empty();
}

} // namespace

int32_t PluginInterface::AddBooleanSample(const std::string &name, int32_t sample)
{
    if (HISTOGRAM_UNLIKELY(IsNullName(name))) {
        AP_ERROR_LOG("AddBooleanSample: invalid name");
        return -1;
    }

    if (HISTOGRAM_UNLIKELY(!IsValidBooleanSample(sample))) {
        AP_ERROR_LOG("AddBooleanSample: invalid sample=%{public}d", sample);
        return -1;
    }

    IHistogramPlugin *plugin = GetPluginFast();
    if (HISTOGRAM_UNLIKELY(plugin == nullptr)) {
        AP_ERROR_LOG("AddBooleanSample: plugin unavailable");
        return -1;
    }

    return plugin->AddBooleanSample(name.c_str(), sample);
}

int32_t PluginInterface::AddEnumSample(const std::string &name, int32_t sample, int32_t boundary)
{
    if (HISTOGRAM_UNLIKELY(IsNullName(name))) {
        AP_ERROR_LOG("AddEnumSample: invalid name");
        return -1;
    }

    if (HISTOGRAM_UNLIKELY(!IsValidEnumSample(sample, boundary))) {
        AP_ERROR_LOG("AddEnumSample: invalid sample=%{public}d, boundary=%{public}d",
            sample, boundary);
        return -1;
    }

    IHistogramPlugin *plugin = GetPluginFast();
    if (HISTOGRAM_UNLIKELY(plugin == nullptr)) {
        return -1;
    }

    return plugin->AddEnumSample(name.c_str(), sample, boundary);
}

int32_t PluginInterface::AddCountSample(
    const std::string &name,
    int32_t sample,
    int32_t min,
    int32_t max,
    size_t bucketCount)
{
    if (HISTOGRAM_UNLIKELY(IsNullName(name))) {
        AP_ERROR_LOG("AddCountSample: invalid name");
        return -1;
    }

    if (HISTOGRAM_UNLIKELY(!IsValidSample(sample))) {
        AP_ERROR_LOG("AddCountSample: invalid sample=%{public}d", sample);
        return -1;
    }

    if (HISTOGRAM_UNLIKELY(!IsValidRange(min, max))) {
        AP_ERROR_LOG("AddCountSample: invalid range min=%{public}d max=%{public}d",
            min, max);
        return -1;
    }

    if (HISTOGRAM_UNLIKELY(!IsValidBucketCount(bucketCount))) {
        AP_ERROR_LOG("AddCountSample: invalid bucketCount=%{public}zu", bucketCount);
        return -1;
    }

    IHistogramPlugin *plugin = GetPluginFast();
    if (HISTOGRAM_UNLIKELY(plugin == nullptr)) {
        return -1;
    }

    return plugin->AddCountSample(name.c_str(), sample, min, max, bucketCount);
}

int32_t PluginInterface::AddTimeSample(const std::string &name, int32_t sample)
{
    if (HISTOGRAM_UNLIKELY(IsNullName(name))) {
        AP_ERROR_LOG("AddTimeSample: invalid name");
        return -1;
    }

    if (HISTOGRAM_UNLIKELY(!IsValidSample(sample))) {
        AP_ERROR_LOG("AddTimeSample: invalid sample=%{public}d", sample);
        return -1;
    }

    IHistogramPlugin *plugin = GetPluginFast();
    if (HISTOGRAM_UNLIKELY(plugin == nullptr)) {
        return -1;
    }

    return plugin->AddTimeSample(name.c_str(), sample);
}

int32_t PluginInterface::AddPercentageSample(const std::string &name, int32_t sample)
{
    if (HISTOGRAM_UNLIKELY(IsNullName(name))) {
        AP_ERROR_LOG("AddPercentageSample: invalid name");
        return -1;
    }

    if (HISTOGRAM_UNLIKELY(sample < 0 || sample > 100)) {
        AP_ERROR_LOG("AddPercentageSample: invalid sample=%{public}d", sample);
        return -1;
    }

    IHistogramPlugin *plugin = GetPluginFast();
    if (HISTOGRAM_UNLIKELY(plugin == nullptr)) {
        return -1;
    }

    return plugin->AddPercentageSample(name.c_str(), sample);
}

} // namespace histogram
} // namespace OHOS