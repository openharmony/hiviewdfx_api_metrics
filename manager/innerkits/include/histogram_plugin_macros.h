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

// ---------------------------- 宏定义 ----------------------------
#define HISTOGRAM_BOOLEAN(name, sample)                                                   \
    do {                                                                                  \
        auto _ret = OHOS::histogram::GetHistogramPlugin().AddBooleanSample(name, sample); \
        if (_ret == -1) {                                                                 \
            AP_ERROR_LOG("HISTOGRAM_BOOLEAN: error for %{public}s", name); \
        }                                                                                 \
    } while (0)

#define HISTOGRAM_ENUMERATION(name, sample, boundary)                                            \
    do {                                                                                         \
        auto _ret = OHOS::histogram::GetHistogramPlugin().AddEnumSample(name, sample, boundary); \
        if (_ret == -1) {                                                                        \
            AP_ERROR_LOG("HISTOGRAM_ENUMERATION: error for %{public}s", name);    \
        }                                                                                        \
    } while (0)

#define HISTOGRAM_CUSTOM_COUNTS(name, sample, min, max, bucket_count)                                           \
    do {                                                                                                        \
        auto _ret = OHOS::histogram::GetHistogramPlugin().AddCountSample(name, sample, min, max, bucket_count); \
        if (_ret == -1) {                                                                                       \
            AP_ERROR_LOG("HISTOGRAM_CUSTOM_COUNTS: error for %{public}s", name);                 \
        }                                                                                                       \
    } while (0)

#define HISTOGRAM_TIMES(name, sample)                                                   \
    do {                                                                                \
        auto _ret = OHOS::histogram::GetHistogramPlugin().AddTimeSample(name, sample);  \
        if (_ret == -1) {                                                               \
            AP_ERROR_LOG("HISTOGRAM_TIMES: error for %{public}s", name); \
        }                                                                               \
    } while (0)

#define HISTOGRAM_PERCENTAGE(name, sample)                                                   \
    do {                                                                                     \
        auto _ret = OHOS::histogram::GetHistogramPlugin().AddPercentageSample(name, sample); \
        if (_ret == -1) {                                                                    \
            AP_ERROR_LOG("HISTOGRAM_PERCENTAGE: error for %{public}s", name); \
        }                                                                                    \
    } while (0)

#endif  // HISTOGRAM_PLUGIN_MACROS_H