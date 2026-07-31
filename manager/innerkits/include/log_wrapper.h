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

#ifndef HIVIEWDFX_API_METRICS_HISTOGRAM_LOG_WRAPPER_H
#define HIVIEWDFX_API_METRICS_HISTOGRAM_LOG_WRAPPER_H

#include "hilog/log.h"

namespace OHOS::histogram {

inline constexpr OHOS::HiviewDFX::HiLogLabel HISTOGRAM_LOG_LABEL = {
    LOG_CORE,
    0xD002D34,
    "HistogramPlugin"
};

} // namespace OHOS::histogram

#define HISTOGRAM_DEBUG_LOG(fmt, ...) \
    OHOS::HiviewDFX::HiLog::Debug(OHOS::histogram::HISTOGRAM_LOG_LABEL, fmt, ##__VA_ARGS__)

#define HISTOGRAM_INFO_LOG(fmt, ...) \
    OHOS::HiviewDFX::HiLog::Info(OHOS::histogram::HISTOGRAM_LOG_LABEL, fmt, ##__VA_ARGS__)

#define HISTOGRAM_WARN_LOG(fmt, ...) \
    OHOS::HiviewDFX::HiLog::Warn(OHOS::histogram::HISTOGRAM_LOG_LABEL, fmt, ##__VA_ARGS__)

#define HISTOGRAM_ERROR_LOG(fmt, ...) \
    OHOS::HiviewDFX::HiLog::Error(OHOS::histogram::HISTOGRAM_LOG_LABEL, fmt, ##__VA_ARGS__)

#endif