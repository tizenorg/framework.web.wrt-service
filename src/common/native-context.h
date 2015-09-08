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

#ifndef WRT_COMMON_NATIVE_CONTEXT_H_
#define WRT_COMMON_NATIVE_CONTEXT_H_

#include <string>
#include <map>

namespace wrt {
namespace common {

class NativeContext {
public:
    static NativeContext* GetInstance();

    int AddCallbackToMap(void* callback);
    void InvokeCallback(int callback_handle, const std::string& data);

    void AddEventToMap(const std::string& event, void* callback);
    void RemoveEventFromMap(const std::string& event, void* callback);
    void FireEvent(const std::string& event, const std::string& data);
private:
    NativeContext();
    virtual ~NativeContext();

    void ClearCallbackMap();
    void ClearEventMap();

    int newHandle();

    void dump();

    typedef std::map<int, void*> CallbackMap;
    CallbackMap async_callback_map_;

    typedef std::multimap<std::string, void*> EventMultiMap;
    EventMultiMap event_map_;
};

} // namespace common
} // namespace wrt

#endif // WRT_COMMON_NATIVE_CONTEXT_H_
