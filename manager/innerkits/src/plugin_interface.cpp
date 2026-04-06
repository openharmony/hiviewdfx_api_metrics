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

namespace OHOS {
namespace histogram {

int32_t PluginInterface::AddBooleanSample(const std::string &name, int32_t sample)
{
    auto manager = PluginManager::GetInstance();
    if (!manager->LazyLoadPlugin()) {
        AP_ERROR_LOG("AddBooleanSample: failed to load plugin");
        return -1;
    }

    auto plugin = manager->GetPlugin();
    if (!plugin) {
        AP_INFO_LOG(
            "AddBooleanSample: No Plugin, name=%{public}s, sample=%{public}d", name.c_str(), static_cast<int>(sample));
        return -1;
    }

    AP_INFO_LOG("AddBooleanSample: Plugin Called");
    return plugin->AddBooleanSample(name, sample);
}

int32_t PluginInterface::AddEnumSample(const std::string &name, int32_t sample, int32_t boundary)
{
    auto manager = PluginManager::GetInstance();
    if (!manager->LazyLoadPlugin()) {
        AP_ERROR_LOG("AddEnumSample: failed to load plugin");
        return -1;
    }

    auto plugin = manager->GetPlugin();
    if (!plugin) {
        AP_INFO_LOG("AddEnumSample: No Plugin, name=%{public}s, sample=%{public}d, boundary=%{public}d",
            name.c_str(),
            sample,
            boundary);
        return -1;
    }

    AP_INFO_LOG("AddEnumSample: Plugin Called");
    return plugin->AddEnumSample(name, sample, boundary);
}

const size_t COUNT_MIN_BUCKET_COUNT = 3;

int32_t PluginInterface::AddCountSample(
    const std::string &name, int32_t sample, int32_t min, int32_t max, size_t bucketCount)
{
    if (bucketCount < COUNT_MIN_BUCKET_COUNT) {
        AP_ERROR_LOG("AddCountSample: invalid bucketCount=%{public}zu, must be >= 3", bucketCount);
        return -1;
    }

    auto manager = PluginManager::GetInstance();
    if (!manager->LazyLoadPlugin()) {
        AP_ERROR_LOG("AddCountSample: failed to load plugin");
        return -1;
    }

    auto plugin = manager->GetPlugin();
    if (!plugin) {
        AP_INFO_LOG("AddCountSample: No Plugin, name=%{public}s, sample=%{public}d, min=%{public}d, "
                    "max=%{public}d, bucketCount=%{public}zu",
            name.c_str(),
            sample,
            min,
            max,
            bucketCount);
        return -1;
    }

    AP_INFO_LOG("AddCountSample: Plugin Called");
    return plugin->AddCountSample(name, sample, min, max, bucketCount);
}

int32_t PluginInterface::AddTimeSample(const std::string &name, int32_t sample)
{
    auto manager = PluginManager::GetInstance();
    if (!manager->LazyLoadPlugin()) {
        AP_ERROR_LOG("AddTimeSample: failed to load plugin");
        return -1;
    }

    auto plugin = manager->GetPlugin();
    if (!plugin) {
        AP_INFO_LOG("AddTimeSample: No Plugin, name=%{public}s, sample=%{public}lld",
            name.c_str(),
            static_cast<long long>(sample));
        return -1;
    }

    AP_INFO_LOG("AddTimeSample: Plugin Called");
    return plugin->AddTimeSample(name, sample);
}

int32_t PluginInterface::AddPercentageSample(const std::string &name, int32_t sample)
{
    auto manager = PluginManager::GetInstance();
    if (!manager->LazyLoadPlugin()) {
        AP_ERROR_LOG("AddPercentageSample: failed to load plugin");
        return -1;
    }

    auto plugin = manager->GetPlugin();
    if (!plugin) {
        AP_INFO_LOG("AddPercentageSample: No Plugin, name=%{public}s, sample=%{public}d", name.c_str(), sample);
        return -1;
    }

    AP_INFO_LOG("AddPercentageSample: Plugin Called");
    return plugin->AddPercentageSample(name, sample);
}

}  // namespace histogram
}  // namespace OHOS