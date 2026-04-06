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

#ifndef HISTOGRAM_PLUGIN_MACROS_H
#define HISTOGRAM_PLUGIN_MACROS_H

#include "plugin_interface.h"
#include "log_wrapper.h"

namespace OHOS::histogram {

// 全局单例对象
inline PluginInterface &GetHistogramPlugin()
{
    static PluginInterface instance;
    return instance;
}

}  // namespace OHOS::histogram

// ---------------------------- 内联函数 ----------------------------
inline void HistogramBoolean(const char* name, bool sample)
{
    auto ret = OHOS::histogram::GetHistogramPlugin().AddBooleanSample(name, sample);
    if (ret == -1) {
        AP_ERROR_LOG("HistogramBoolean: no plugin registered for %{public}s", name);
    }
}

inline void HistogramEnumeration(const char* name, int sample, int boundary)
{
    auto ret = OHOS::histogram::GetHistogramPlugin().AddEnumSample(name, sample, boundary);
    if (ret == -1) {
        AP_ERROR_LOG("HistogramEnumeration: no plugin registered for %{public}s", name);
    }
}

inline void HistogramCustomCounts(const char* name, double sample, double min, double max, int bucketCount)
{
    auto ret = OHOS::histogram::GetHistogramPlugin().AddCountSample(name, sample, min, max, bucketCount);
    if (ret == -1) {
        AP_ERROR_LOG("HistogramCustomCounts: no plugin registered for %{public}s", name);
    }
}

inline void HistogramTimes(const char* name, int64_t sample)
{
    auto ret = OHOS::histogram::GetHistogramPlugin().AddTimeSample(name, sample);
    if (ret == -1) {
        AP_ERROR_LOG("HistogramTimes: no plugin registered for %{public}s", name);
    }
}

inline void HistogramPercentage(const char* name, double sample)
{
    auto ret = OHOS::histogram::GetHistogramPlugin().AddPercentageSample(name, sample);
    if (ret == -1) {
        AP_ERROR_LOG("HistogramPercentage: no plugin registered for %{public}s", name);
    }
}

#endif  // HISTOGRAM_PLUGIN_MACROS_H