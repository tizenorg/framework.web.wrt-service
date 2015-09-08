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

#ifndef WRT_SERVICE_NODE_NATIVE_PLUGIN_LOADER_H_
#define WRT_SERVICE_NODE_NATIVE_PLUGIN_LOADER_H_

#include <map>
#include <string>

#include <native-plugin.h>

using namespace wrt::common;

namespace wrt {
namespace service {

class NativePluginLoader {
public:
    static NativePluginLoader* GetInstance();

    NativePlugin* Load(std::string& name);

private:
    typedef std::map<std::string, NativePlugin*> PluginMap;
    PluginMap plugin_map_;

};

} // namespace service
} // namespace wrt

#endif // WRT_SERVICE_NODE_NATIVE_PLUGIN_LOADER_H_
