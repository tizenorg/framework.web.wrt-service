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

#include <v8.h>
#include <node.h>

#include <native-plugin.h>
#include <native-context.h>

#include "native-node.h"
#include "native-plugin-loader.h"

using namespace v8;
using namespace node;
using namespace wrt::common;

namespace wrt {
namespace service {

static Handle<String> ToJSON(const Handle<Value> object)
{
    HandleScope scope;

    Local<Object> global = Context::GetCurrent()->Global();
    Local<Object> JSON = global->Get(String::New("JSON"))->ToObject();
    Local<Function> JSON_stringify
            = Local<Function>::Cast(JSON->Get(String::New("stringify")));

    Local<Value> args[] = { Local<Value>::New(object) };
    Local<String> result = Local<String>::Cast(JSON_stringify->Call(JSON, 1, args));
    return scope.Close(result);
}

void NativeNode::Init(Handle<Object> target)
{
    HandleScope scope;

    target->Set(String::NewSymbol("call"),
        FunctionTemplate::New(Call)->GetFunction());

    target->Set(String::NewSymbol("callSync"),
        FunctionTemplate::New(CallSync)->GetFunction());

    target->Set(String::NewSymbol("addListener"),
        FunctionTemplate::New(AddListener)->GetFunction());

    target->Set(String::NewSymbol("removeListener"),
        FunctionTemplate::New(RemoveListener)->GetFunction());
}

Handle<Value> NativeNode::Call(const Arguments& args)
{
    HandleScope scope;

    if (args.Length() < 1 || !args[0]->IsObject()) {
        return ThrowException(Exception::TypeError(String::New("First argument must be an object")));
    }

    if (args.Length() < 2 || !args[1]->IsFunction()) {
        return ThrowException(Exception::TypeError(String::New("Second argument must be an function")));
    }

    Handle<Object> obj = Handle<Object>::Cast(args[0]);
    Persistent<Function> func = Persistent<Function>::New(Handle<Function>::Cast(args[1]));

    if (obj->Has(String::New("module"))) {
        // Load module
        std::string module(*String::Utf8Value(obj->Get(String::New("module"))->ToString()));
        NativePlugin* plugin = NativePluginLoader::GetInstance()->Load(module);
        if (!plugin) {
            return ThrowException(Exception::TypeError(String::New("Plugin cannot be loaded")));
        }

        if (plugin && obj->Has(String::New("data"))) {
            std::string data_str;
            Handle<Value> data = obj->Get(String::New("data"));
            if (data->IsObject()) {
                data_str = *(String::Utf8Value(ToJSON(data)));
            } else {
                data_str = *(String::Utf8Value(data->ToString()));
            }

            // Register callback
            int handle = NativeContext::GetInstance()->AddCallbackToMap(static_cast<void*>(*func));

            // Invoke method
            std::string ret_str = plugin->OnCall(data_str, handle);

            // Return
            return scope.Close(String::New(ret_str.c_str()));
        }
    }

    return Undefined();
}

Handle<Value> NativeNode::CallSync(const Arguments& args)
{
    HandleScope scope;

    if (args.Length() < 1 || !args[0]->IsObject()) {
        return ThrowException(Exception::TypeError(String::New("Argument must be an object")));
    }

    Handle<Object> obj = Handle<Object>::Cast(args[0]);

    if (obj->Has(String::New("module"))) {
        // Load module
        std::string module(*String::Utf8Value(obj->Get(String::New("module"))->ToString()));
        NativePlugin* plugin = NativePluginLoader::GetInstance()->Load(module);
        if (!plugin) {
            return ThrowException(Exception::TypeError(String::New("Plugin cannot be loaded")));
        }

        if (plugin && obj->Has(String::New("data"))) {
            std::string data_str;
            Handle<Value> data = obj->Get(String::New("data"));
            if (data->IsObject()) {
                data_str = *(String::Utf8Value(ToJSON(data)));
            } else {
                data_str = *(String::Utf8Value(data->ToString()));
            }

            // Invoke method
            std::string ret_str = plugin->OnCallSync(data_str);

            // Return
            return scope.Close(String::New(ret_str.c_str()));
        }
    }

    return Undefined();
}

Handle<Value> NativeNode::AddListener(const Arguments& args)
{
    HandleScope scope;

    if (args.Length() < 1 || !args[0]->IsString()) {
        return ThrowException(Exception::TypeError(String::New("First argument must be an string")));
    }

    if (args.Length() < 2 || !args[1]->IsFunction()) {
        return ThrowException(Exception::TypeError(String::New("Second argument must be an function")));
    }

    std::string event(*String::Utf8Value(Handle<String>::Cast(args[0])));
    Persistent<Function> func = Persistent<Function>::New(Handle<Function>::Cast(args[1]));

    NativeContext::GetInstance()->AddEventToMap(event, static_cast<void*>(*func));

    return Undefined();
}

Handle<Value> NativeNode::RemoveListener(const Arguments& args)
{
    HandleScope scope;

    if (args.Length() < 1 || !args[0]->IsString()) {
        return ThrowException(Exception::TypeError(String::New("First argument must be an string")));
    }

    if (args.Length() < 2 || !args[1]->IsFunction()) {
        return ThrowException(Exception::TypeError(String::New("Second argument must be an function")));
    }

    std::string event(*String::Utf8Value(Handle<String>::Cast(args[0])));
    Local<Function> func = Local<Function>::New(Handle<Function>::Cast(args[1]));

    NativeContext::GetInstance()->RemoveEventFromMap(event, static_cast<void*>(*func));

    return Undefined();
}

} // namespace service
} // namespace wrt

extern "C" {
    void static NodeInit(Handle<Object> target) {
        wrt::service::NativeNode::Init(target);
    }

    NODE_MODULE(native, NodeInit);
}
