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

#ifndef HISTOGRAM_UNLIKELY
#define HISTOGRAM_UNLIKELY(x) __builtin_expect(!!(x), 0)
#endif

namespace OHOS::histogram {

inline PluginInterface &GetHistogramPlugin()
{
    static PluginInterface instance;
    return instance;
}

} // namespace OHOS::histogram

#define HISTOGRAM_BOOLEAN(name, sample)                                                   \
    do {                                                                                  \
        int32_t _ret = OHOS::histogram::GetHistogramPlugin().AddBooleanSample(name, sample); \
        if (HISTOGRAM_UNLIKELY(_ret != 0)) {                                               \
            AP_ERROR_LOG("HISTOGRAM_BOOLEAN failed, name=%{public}s", name);              \
        }                                                                                 \
    } while (0)

#define HISTOGRAM_ENUMERATION(name, sample, boundary)                                    \
    do {                                                                                 \
        int32_t _ret = OHOS::histogram::GetHistogramPlugin().AddEnumSample(               \
            name, sample, boundary);                                                     \
        if (HISTOGRAM_UNLIKELY(_ret != 0)) {                                              \
            AP_ERROR_LOG("HISTOGRAM_ENUMERATION failed, name=%{public}s", name);         \
        }                                                                                \
    } while (0)

#define HISTOGRAM_CUSTOM_COUNTS(name, sample, min, max, bucket_count)                    \
    do {                                                                                 \
        int32_t _ret = OHOS::histogram::GetHistogramPlugin().AddCountSample(              \
            name, sample, min, max, bucket_count);                                       \
        if (HISTOGRAM_UNLIKELY(_ret != 0)) {                                              \
            AP_ERROR_LOG("HISTOGRAM_CUSTOM_COUNTS failed, name=%{public}s", name);       \
        }                                                                                \
    } while (0)

#define HISTOGRAM_TIMES(name, sample)                                                    \
    do {                                                                                 \
        int32_t _ret = OHOS::histogram::GetHistogramPlugin().AddTimeSample(name, sample); \
        if (HISTOGRAM_UNLIKELY(_ret != 0)) {                                              \
            AP_ERROR_LOG("HISTOGRAM_TIMES failed, name=%{public}s", name);               \
        }                                                                                \
    } while (0)

#define HISTOGRAM_PERCENTAGE(name, sample)                                               \
    do {                                                                                 \
        int32_t _ret = OHOS::histogram::GetHistogramPlugin().AddPercentageSample(name, sample); \
        if (HISTOGRAM_UNLIKELY(_ret != 0)) {                                              \
            AP_ERROR_LOG("HISTOGRAM_PERCENTAGE failed, name=%{public}s", name);          \
        }                                                                                \
    } while (0)

#endif // HISTOGRAM_PLUGIN_MACROS_H