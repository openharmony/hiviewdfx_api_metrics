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

#include <limits>

namespace OHOS {
namespace histogram {

namespace {

constexpr int32_t SAMPLE_MIN = 0;
constexpr int32_t SAMPLE_MAX = std::numeric_limits<int32_t>::max();
constexpr size_t MIN_BUCKET_COUNT = 2;

inline bool IsValidSample(int32_t sample)
{
    return sample >= SAMPLE_MIN && sample < SAMPLE_MAX;
}

inline bool IsValidBucketCount(size_t bucketCount)
{
    return bucketCount >= MIN_BUCKET_COUNT;
}

inline bool IsValidRange(int32_t min, int32_t max)
{
    return min < max;
}

inline IHistogramPlugin* GetPluginFast()
{
    auto manager = PluginManager::GetInstance();

    IHistogramPlugin* plugin = manager->GetPlugin();
    if (plugin != nullptr) {
        return plugin;
    }

    if (!manager->LazyLoadPlugin()) {
        return nullptr;
    }

    return manager->GetPlugin();
}

} // namespace

int32_t PluginInterface::AddBooleanSample(const std::string &name, int32_t sample)
{
    if (!IsValidSample(sample)) {
        AP_ERROR_LOG("AddBooleanSample: invalid sample=%{public}d", sample);
        return -1;
    }

    IHistogramPlugin* plugin = GetPluginFast();
    if (!plugin) {
        return -1;
    }

    return plugin->AddBooleanSample(name, sample);
}

int32_t PluginInterface::AddEnumSample(const std::string &name, int32_t sample, int32_t boundary)
{
    if (!IsValidSample(sample)) {
        AP_ERROR_LOG("AddEnumSample: invalid sample=%{public}d", sample);
        return -1;
    }

    IHistogramPlugin* plugin = GetPluginFast();
    if (!plugin) {
        return -1;
    }

    return plugin->AddEnumSample(name, sample, boundary);
}

int32_t PluginInterface::AddCountSample(
    const std::string &name, int32_t sample, int32_t min, int32_t max, size_t bucketCount)
{
    if (!IsValidSample(sample)) {
        AP_ERROR_LOG("AddCountSample: invalid sample=%{public}d", sample);
        return -1;
    }

    if (!IsValidBucketCount(bucketCount)) {
        AP_ERROR_LOG("AddCountSample: invalid bucketCount");
        return -1;
    }

    if (!IsValidRange(min, max)) {
        AP_ERROR_LOG("AddCountSample: invalid range, min=%{public}d, max=%{public}d", min, max);
        return -1;
    }

    IHistogramPlugin* plugin = GetPluginFast();
    if (!plugin) {
        return -1;
    }

    return plugin->AddCountSample(name, sample, min, max, bucketCount);
}

int32_t PluginInterface::AddTimeSample(const std::string &name, int32_t sample)
{
    if (!IsValidSample(sample)) {
        AP_ERROR_LOG("AddTimeSample: invalid sample=%{public}d", sample);
        return -1;
    }

    IHistogramPlugin* plugin = GetPluginFast();
    if (!plugin) {
        return -1;
    }

    return plugin->AddTimeSample(name, sample);
}

int32_t PluginInterface::AddPercentageSample(const std::string &name, int32_t sample)
{
    if (!IsValidSample(sample)) {
        AP_ERROR_LOG("AddPercentageSample: invalid sample=%{public}d", sample);
        return -1;
    }

    IHistogramPlugin* plugin = GetPluginFast();
    if (!plugin) {
        return -1;
    }

    return plugin->AddPercentageSample(name, sample);
}

} // namespace histogram
} // namespace OHOS