/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include <dlfcn.h>

#include <dlog.h>

#include "native-plugin-loader.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "WRT_SERVICE"

namespace wrt {
namespace service {

namespace {
    const char* kPluginPath = "/usr/lib/wrt-plugins";
    const char* kPluginPrefix = "lib";
    const char* kPluginSuffix = ".so";
}

NativePluginLoader* NativePluginLoader::GetInstance()
{
    static NativePluginLoader instance;
    return &instance;
}

NativePlugin* NativePluginLoader::Load(std::string& name)
{
    std::string pluginPath(kPluginPath);
    pluginPath.append("/");
    pluginPath.append(kPluginPrefix);
    pluginPath.append(name);
    pluginPath.append(kPluginSuffix);

    PluginMap::iterator it;

    NativePlugin* plugin = NULL;
    void* handle;

    it = plugin_map_.find(name);
    if (it == plugin_map_.end()) {
        // Open a plugin object
        handle = dlopen(pluginPath.c_str(), RTLD_NOW | RTLD_GLOBAL);
        if (!handle) {
            LOGE("Plugin Loading is failed. %s", dlerror());
            return NULL;
        }
        // Clear errors
        dlerror();

        // Get a factory Function
        create_native_plugin_t* createFunc
                = (create_native_plugin_t *)dlsym(handle, "create_native_plugin");
        if (!createFunc) {
            LOGE("Plugin Loading is failed. %s", dlerror());
            dlclose(handle);
            return NULL;
        }

        // Create a new plugin instance
        plugin = createFunc();
        if (!plugin) {
            LOGE("Plugin initialization is failed.");
            fprintf(stderr, "Plugin initialization is failed.");
            dlclose(handle);
            return NULL;
        }

        // Call "OnLoad" callback
        plugin->OnLoad();

        plugin_map_[name] = plugin;

    } else {
        plugin = it->second;
    }

    return plugin;
}

} // namespace service
} // namespace wrt

