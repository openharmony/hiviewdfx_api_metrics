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

#ifndef HISTOGRAM_PLUGIN_INTERFACE_H
#define HISTOGRAM_PLUGIN_INTERFACE_H

#include "ihistogram_plugin.h"
#include "plugin_manager.h"
#include "log_wrapper.h"

#include <cstdint>
#include <string>
#include <mutex>
#include <unistd.h>

namespace OHOS {
namespace histogram {

class PluginInterface {
public:
    PluginInterface() = default;
    ~PluginInterface() = default;

    int32_t AddBooleanSample(const std::string &name, int32_t sample);
    int32_t AddEnumSample(const std::string &name, int32_t sample, int32_t boundary);
    int32_t AddCountSample(const std::string &name, int32_t sample, int32_t min, int32_t max, size_t bucketCount);
    int32_t AddTimeSample(const std::string &name, int32_t sample);
    int32_t AddPercentageSample(const std::string &name, int32_t sample);
};

} // namespace histogram
} // namespace OHOS

#endif // HISTOGRAM_PLUGIN_INTERFACE_H