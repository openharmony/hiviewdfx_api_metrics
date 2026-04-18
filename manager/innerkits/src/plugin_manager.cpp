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

#include "plugin_manager.h"

#include <dlfcn.h>

#include "log_wrapper.h"

namespace OHOS {
namespace histogram {

using create_plugin_func_t = IHistogramPlugin *(*)();

std::shared_ptr<PluginManager> PluginManager::instance_ = nullptr;
std::mutex PluginManager::instanceMutex_;

std::shared_ptr<PluginManager> PluginManager::GetInstance()
{
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> lock(instanceMutex_);
        if (instance_ == nullptr) {
            instance_ = std::make_shared<PluginManager>();
        }
    }
    return instance_;
}

PluginManager::PluginManager()
{
}

PluginManager::~PluginManager()
{
    UnloadAllPlugins();
}

bool PluginManager::LazyLoadPlugin()
{
    return LoadPluginIfNeeded(HISTOGRAM_PLUGIN_SO_PATH);
}

bool PluginManager::LoadPluginIfNeeded(const std::string &path)
{
    if (pluginHasInit_) {
        return true;
    }

    std::lock_guard<std::mutex> lock(mutex_);

    if (pluginHasInit_) {
        return true;
    }

    return LoadPluginLocked(path);
}

bool PluginManager::LoadPlugin(const std::string &path)
{
    std::lock_guard<std::mutex> lock(mutex_);
    return LoadPluginLocked(path);
}

bool PluginManager::LoadPluginLocked(const std::string &path)
{
    if (path.empty()) {
        AP_ERROR_LOG("plugin_manager: plugin path is empty");
        return false;
    }

    if (pluginHasInit_) {
        return true;
    }

    dlerror();
    void *handle = dlopen(path.c_str(), RTLD_NOW);
    if (handle == nullptr) {
        AP_ERROR_LOG("plugin_manager: dlopen failed, path=%{public}s, err=%{public}s",
            path.c_str(),
            dlerror());
        return false;
    }

    pluginHandle_ = handle;
    pluginPath_ = path;

    if (!ResolveAndCreatePluginLocked(path)) {
        AP_ERROR_LOG("plugin_manager: load plugin failed, path=%{public}s", path.c_str());
        return false;
    }

    AP_DEBUG_LOG("plugin_manager: plugin loaded successfully, path=%{public}s", path.c_str());
    return true;
}

bool PluginManager::ResolveAndCreatePlugin(const std::string &path)
{
    std::lock_guard<std::mutex> lock(mutex_);
    return ResolveAndCreatePluginLocked(path);
}

bool PluginManager::ResolveAndCreatePluginLocked(const std::string &path)
{
    if (pluginHandle_ == nullptr) {
        AP_ERROR_LOG("plugin_manager: plugin handle is nullptr, path=%{public}s", path.c_str());
        return false;
    }

    dlerror();
    auto createFunc = reinterpret_cast<create_plugin_func_t>(dlsym(pluginHandle_, CREATE_HISTOGRAM_PLUGIN.c_str()));
    const char *err = dlerror();
    if (err != nullptr || createFunc == nullptr) {
        AP_ERROR_LOG("plugin_manager: dlsym %{public}s failed, err=%{public}s",
            CREATE_HISTOGRAM_PLUGIN.c_str(),
            err == nullptr ? "unknown error" : err);

        if (pluginHandle_ != nullptr) {
            dlclose(pluginHandle_);
            pluginHandle_ = nullptr;
        }
        plugin_ = nullptr;
        pluginHasInit_ = false;
        pluginPath_.clear();
        return false;
    }

    IHistogramPlugin *plugin = createFunc();
    if (plugin == nullptr) {
        AP_ERROR_LOG("plugin_manager: %{public}s returned nullptr", CREATE_HISTOGRAM_PLUGIN.c_str());

        if (pluginHandle_ != nullptr) {
            dlclose(pluginHandle_);
            pluginHandle_ = nullptr;
        }
        plugin_ = nullptr;
        pluginHasInit_ = false;
        pluginPath_.clear();
        return false;
    }

    plugin_ = plugin;
    pluginHasInit_ = true;
    return true;
}

void PluginManager::RegisterPlugin(IHistogramPlugin *plugin)
{
    if (plugin == nullptr) {
        AP_ERROR_LOG("plugin_manager: RegisterPlugin failed, plugin is nullptr");
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    plugin_ = plugin;
    pluginHasInit_ = true;
    AP_DEBUG_LOG("plugin_manager: plugin registered successfully");
}

IHistogramPlugin *PluginManager::GetPlugin()
{
    return plugin_;
}

bool PluginManager::UnloadPlugin()
{
    void *handle = nullptr;
    std::string path;

    {
        std::lock_guard<std::mutex> lock(mutex_);

        if (!pluginHasInit_ && pluginHandle_ == nullptr) {
            return true;
        }

        handle = pluginHandle_;
        path = pluginPath_;

        plugin_ = nullptr;
        pluginHandle_ = nullptr;
        pluginHasInit_ = false;
        pluginPath_.clear();
    }

    if (handle == nullptr) {
        return true;
    }

    int ret = dlclose(handle);
    if (ret != 0) {
        AP_ERROR_LOG("plugin_manager: dlclose failed, path=%{public}s, err=%{public}s",
            path.c_str(),
            dlerror());
        return false;
    }

    AP_DEBUG_LOG("plugin_manager: plugin unloaded, path=%{public}s", path.c_str());
    return true;
}

void PluginManager::UnloadAllPlugins()
{
    if (!UnloadPlugin()) {
        AP_ERROR_LOG("plugin_manager: UnloadAllPlugins failed");
        return;
    }
}

}  // namespace histogram
}  // namespace OHOS