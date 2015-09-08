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

#ifndef WRT_SERIVCE_NODE_NATIVE_NODE_H_
#define WRT_SERIVCE_NODE_NATIVE_NODE_H_

#include <v8.h>

using namespace v8;

namespace wrt {
namespace service {

class NativeNode {
public:
    static void Init(Handle<Object> target);

    static Handle<Value> Call(const Arguments& args);
    static Handle<Value> CallSync(const Arguments& args);
    static Handle<Value> AddListener(const Arguments& args);
    static Handle<Value> RemoveListener(const Arguments& args);

};

} // namespace service
} // namespace wrt

#endif // WRT_SERIVCE_NODE_NATIVE_NODE_H_
