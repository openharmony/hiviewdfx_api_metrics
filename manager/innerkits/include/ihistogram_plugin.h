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

#ifndef HISTOGRAM_IHISTOGRAM_PLUGIN_H
#define HISTOGRAM_IHISTOGRAM_PLUGIN_H

#include <cstddef>
#include <cstdint>
#include <string>

namespace OHOS {
namespace histogram {

/*
 * 插件协议常量定义
 */
#if (defined(__aarch64__) || defined(__x86_64__))
inline const std::string HISTOGRAM_PLUGIN_SO_PATH = "/system/lib64/platformsdk/libhistogram.z.so";
#else
inline const std::string HISTOGRAM_PLUGIN_SO_PATH = "/system/lib/platformsdk/libhistogram.z.so";
#endif

// 插件导出的创建入口函数名
inline const std::string CREATE_HISTOGRAM_PLUGIN = "create_histogram_plugin";

// 插件状态码
static constexpr int32_t HISTOGRAM_PLUGIN_ENABLE = 0;
static constexpr int32_t HISTOGRAM_PLUGIN_DISABLE = -1;

/**
 * @brief Histogram Plugin Interface
 *
 * 所有 Histogram 插件必须实现该接口。
 * PluginManager 只依赖该接口，不依赖具体实现。
 */
class IHistogramPlugin {
public:
    virtual ~IHistogramPlugin() = default;

    /**
     * @brief 枚举型直方图
     *
     * @param histogram_name 直方图名称
     * @param sample 枚举值
     * @param max 枚举上界
     * @return int32_t 执行结果
     */
    virtual int32_t AddEnumSample(const std::string &histogram_name, int32_t sample, int32_t max) = 0;

    /**
     * @brief 时间类型直方图
     *
     * @param histogram_name 直方图名称
     * @param sample 时间值
     * @return int32_t 执行结果
     */
    virtual int32_t AddTimeSample(const std::string &histogram_name, int32_t sample) = 0;

    /**
     * @brief 计数型直方图
     *
     * @param histogram_name 直方图名称
     * @param sample 计数值
     * @param min 最小值
     * @param max 最大值
     * @param bucket_count 桶数量
     * @return int32_t 执行结果
     */
    virtual int32_t AddCountSample(
        const std::string &histogram_name, int32_t sample, int32_t min, int32_t max, size_t bucket_count) = 0;

    /**
     * @brief 布尔型直方图
     *
     * @param histogram_name 直方图名称
     * @param sample 0/1
     * @return int32_t 执行结果
     */
    virtual int32_t AddBooleanSample(const std::string &histogram_name, int32_t sample) = 0;

    /**
     * @brief 百分比直方图
     *
     * @param histogram_name 直方图名称
     * @param sample 百分比值
     * @return int32_t 执行结果
     */
    virtual int32_t AddPercentageSample(const std::string &histogram_name, int32_t sample) = 0;
};

}  // namespace histogram
}  // namespace OHOS

#endif  // HISTOGRAM_IHISTOGRAM_PLUGIN_H