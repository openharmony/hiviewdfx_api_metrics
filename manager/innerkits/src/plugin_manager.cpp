/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 *
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
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

namespace {
using CreatePluginFunc = IHistogramPlugin *(*)();
}

PluginManager &PluginManager::GetInstance()
{
    static PluginManager instance;
    return instance;
}

PluginManager::PluginManager() = default;

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
    if (pluginHasInit_.load(std::memory_order_acquire)) {
        return true;
    }

    std::lock_guard<std::mutex> lock(mutex_);

    if (pluginHasInit_.load(std::memory_order_relaxed)) {
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

    if (pluginHasInit_.load(std::memory_order_relaxed)) {
        return true;
    }

    (void)dlerror();
    void *handle = dlopen(path.c_str(), RTLD_NOW | RTLD_LOCAL);
    if (handle == nullptr) {
        AP_ERROR_LOG("plugin_manager: dlopen failed, path=%{public}s, err=%{public}s",
            path.c_str(), dlerror());
        return false;
    }

    pluginHandle_ = handle;
    pluginPath_ = path;

    if (!ResolveAndCreatePluginLocked(path)) {
        AP_ERROR_LOG("plugin_manager: resolve plugin failed, path=%{public}s", path.c_str());
        (void)dlclose(handle);
        pluginHandle_ = nullptr;
        pluginPath_.clear();
        return false;
    }

    AP_DEBUG_LOG("plugin_manager: plugin loaded successfully, path=%{public}s", path.c_str());
    return true;
}

bool PluginManager::ResolveAndCreatePluginLocked(const std::string &path)
{
    if (pluginHandle_ == nullptr) {
        AP_ERROR_LOG("plugin_manager: plugin handle is nullptr, path=%{public}s", path.c_str());
        return false;
    }

    auto createFunc = reinterpret_cast<CreatePluginFunc>(
        dlsym(pluginHandle_, CREATE_HISTOGRAM_PLUGIN.c_str()));
    if (createFunc == nullptr) {
        const char *err = dlerror();
        AP_ERROR_LOG("plugin_manager: dlsym %{public}s failed, err=%{public}s",
            CREATE_HISTOGRAM_PLUGIN.c_str(),
            err == nullptr ? "unknown error" : err);
        ResetPluginLocked();
        return false;
    }

    IHistogramPlugin *plugin = createFunc();
    if (plugin == nullptr) {
        AP_ERROR_LOG("plugin_manager: create plugin returned nullptr");
        ResetPluginLocked();
        return false;
    }

    plugin_.store(plugin, std::memory_order_release);
    pluginHasInit_.store(true, std::memory_order_release);
    return true;
}

void PluginManager::RegisterPlugin(IHistogramPlugin *plugin)
{
    if (plugin == nullptr) {
        AP_ERROR_LOG("plugin_manager: RegisterPlugin failed, plugin is nullptr");
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    plugin_.store(plugin, std::memory_order_release);
    pluginHasInit_.store(true, std::memory_order_release);
}

bool PluginManager::UnloadPlugin()
{
    void *handle = nullptr;
    std::string path;

    {
        std::lock_guard<std::mutex> lock(mutex_);

        if (!pluginHasInit_.load(std::memory_order_acquire) && pluginHandle_ == nullptr) {
            return true;
        }

        handle = pluginHandle_;
        path = pluginPath_;

        ResetPluginLocked();
    }

    if (handle == nullptr) {
        return true;
    }

    if (dlclose(handle) != 0) {
        AP_ERROR_LOG("plugin_manager: dlclose failed, path=%{public}s, err=%{public}s",
            path.c_str(), dlerror());
        return false;
    }

    return true;
}

void PluginManager::UnloadAllPlugins()
{
    if (!UnloadPlugin()) {
        AP_ERROR_LOG("plugin_manager: UnloadAllPlugins failed");
    }
}

void PluginManager::ResetPluginLocked()
{
    plugin_.store(nullptr, std::memory_order_release);
    pluginHasInit_.store(false, std::memory_order_release);
    pluginHandle_ = nullptr;
    pluginPath_.clear();
}

} // namespace histogram
} // namespace OHOS