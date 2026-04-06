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

#ifndef HISTOGRAM_PLUGIN_MANAGER_H
#define HISTOGRAM_PLUGIN_MANAGER_H

#include <memory>
#include <mutex>
#include <string>

#include "ihistogram_plugin.h"

namespace OHOS {
namespace histogram {

class PluginManager {
public:
    static std::shared_ptr<PluginManager> GetInstance();

    PluginManager();
    ~PluginManager();

    bool LazyLoadPlugin();
    bool LoadPluginIfNeeded(const std::string &path);
    void RegisterPlugin(IHistogramPlugin *plugin);
    IHistogramPlugin *GetPlugin();
    void UnloadAllPlugins();

private:
    bool LoadPlugin(const std::string &path);
    bool ResolveAndCreatePlugin(const std::string &path);
    bool UnloadPlugin();

private:
    static std::shared_ptr<PluginManager> instance_;
    static std::mutex instanceMutex_;

    std::mutex mutex_;
    IHistogramPlugin *plugin_ = nullptr;
    void *pluginHandle_ = nullptr;
    bool pluginHasInit_ = false;
    std::string pluginPath_;
};

}  // namespace histogram
}  // namespace OHOS

#endif  // HISTOGRAM_PLUGIN_MANAGER_H