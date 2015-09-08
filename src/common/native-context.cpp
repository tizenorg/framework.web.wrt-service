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

#include <string>
#include <map>
#include <list>

#include <v8.h>

#include <dlog.h>

#include "native-context.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "WRT"

using namespace v8;

namespace wrt {
namespace common {

NativeContext::NativeContext()
{
}

NativeContext::~NativeContext()
{
    ClearCallbackMap();
    ClearEventMap();
}

NativeContext* NativeContext::GetInstance()
{
    static NativeContext instance;
    return &instance;
}

int NativeContext::newHandle()
{
    static unsigned int handle = 0;
    return (int)++handle;
}

int NativeContext::AddCallbackToMap(void* callback)
{
    int handle = newHandle();
    async_callback_map_.insert(std::make_pair(handle, callback));
    return handle;
}

void NativeContext::InvokeCallback(int handle, const std::string& data)
{
    HandleScope scope;

    CallbackMap::iterator it;
    it = async_callback_map_.find(handle);
    if (it != async_callback_map_.end()) {
        // Invoke a callback
        Persistent<Function> func = static_cast<Function*>(it->second);
        Local<Value> args[] = { Local<Value>::New(String::New(data.c_str())) };
        func->Call(func, 1, args);

        // Remove callback from the list.
        func.Dispose();
        async_callback_map_.erase(it);
    }
}

void NativeContext::AddEventToMap(const std::string& event, void* callback)
{
    HandleScope scope;

    bool exist = false;
    std::pair<EventMultiMap::iterator, EventMultiMap::iterator> iter_pair;

    Local<Function> func_to_add = static_cast<Function*>(callback);

    iter_pair = event_map_.equal_range(event);
    for(EventMultiMap::iterator it = iter_pair.first; it != iter_pair.second; ++it) {
        Persistent<Function> func = static_cast<Function*>(it->second);
        if (func_to_add->Equals(func)) {
            exist = true;
            break;
        }
    }

    if (!exist) {
        event_map_.insert(std::make_pair(event, callback));
    }
}

void NativeContext::RemoveEventFromMap(const std::string& event, void* callback)
{
    HandleScope scope;

    std::pair<EventMultiMap::iterator, EventMultiMap::iterator> iter_pair;

    Local<Function> func_to_remove = static_cast<Function*>(callback);

    iter_pair = event_map_.equal_range(event);
    for(EventMultiMap::iterator it = iter_pair.first; it != iter_pair.second;) {
        Persistent<Function> func = static_cast<Function*>(it->second);
        if (func_to_remove->Equals(func)) {
            func.Dispose();
            event_map_.erase(it++);
        } else {
            ++it;
        }
    }
}

void NativeContext::FireEvent(const std::string& event, const std::string& data)
{
    HandleScope scope;

    std::pair<EventMultiMap::iterator, EventMultiMap::iterator> iter_pair;
    //Temporarily function list
    std::list<Local<Function>> function_snapshot;

    iter_pair = event_map_.equal_range(event);
    for(EventMultiMap::iterator it = iter_pair.first; it != iter_pair.second;++it) {
        Local<Function> func = static_cast<Function*>(it->second);
        // save listener function temporarily
        function_snapshot.push_back(func);
    }
    Local<Value> args[] = { Local<Value>::New(String::New(data.c_str())) };
    for( auto &func : function_snapshot ){
        // perform the call
        func->Call(func, 1, args);
    }
}

void NativeContext::ClearCallbackMap()
{
    CallbackMap::iterator it;
    for (it = async_callback_map_.begin(); it != async_callback_map_.end(); ++it) {
        Persistent<Function> func = static_cast<Function*>(it->second);
        func.Dispose();
    }
    async_callback_map_.clear();
}

void NativeContext::ClearEventMap()
{
    EventMultiMap::iterator it;
    for (it = event_map_.begin(); it != event_map_.end(); ++it) {
        Persistent<Function> func = static_cast<Function*>(it->second);
        func.Dispose();
    }
    event_map_.clear();
}

void NativeContext::dump()
{
    LOGD("Dump [AsyncCallbackMap] Size: %d", async_callback_map_.size());
    LOGD("------------------------------------------------------------");
    CallbackMap::iterator cit;
    for (cit = async_callback_map_.begin(); cit != async_callback_map_.end(); ++cit) {
        LOGD("Key: %d  -->  El: %p", cit->first, cit->second);
    }

    LOGD("Dump [EventMap] Size: %d", event_map_.size());
    LOGD("------------------------------------------------------------");
    EventMultiMap::iterator eit;
    for (eit = event_map_.begin(); eit != event_map_.end(); ++eit) {
        LOGD("Key: %s  -->  El: %p", eit->first.c_str(), eit->second);
    }
    LOGD("------------------------------------------------------------");
}

} // namespace common
} // namespace wrt

