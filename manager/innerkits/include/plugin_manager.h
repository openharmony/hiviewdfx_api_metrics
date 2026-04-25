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


#ifndef PLUGIN_MANAGER_H
#define PLUGIN_MANAGER_H

#include <atomic>
#include <memory>
#include <mutex>
#include <string>

#include "ihistogram_plugin.h"

namespace OHOS {
namespace histogram {

class PluginManager {
public:
    static PluginManager &GetInstance();

    PluginManager();
    ~PluginManager();

    bool LazyLoadPlugin();
    bool LoadPluginIfNeeded(const std::string &path);
    bool LoadPlugin(const std::string &path);

    void RegisterPlugin(IHistogramPlugin *plugin);

    inline IHistogramPlugin *GetPlugin() const
    {
        return plugin_.load(std::memory_order_acquire);
    }

    bool UnloadPlugin();
    void UnloadAllPlugins();

private:
    bool LoadPluginLocked(const std::string &path);
    bool ResolveAndCreatePluginLocked(const std::string &path);
    void ResetPluginLocked();

private:
    std::mutex mutex_;

    void *pluginHandle_ = nullptr;
    std::string pluginPath_;

    std::atomic<IHistogramPlugin *> plugin_ { nullptr };
    std::atomic<bool> pluginHasInit_ { false };
};

} // namespace histogram
} // namespace OHOS

#endif // PLUGIN_MANAGER_H