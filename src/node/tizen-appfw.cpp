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

#include <string.h>
#include <glib.h>
#include <dlog.h>
#include <v8.h>
#include <node.h>

#include "tizen-appfw.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "WRT_SERVICE"

using namespace node;
using namespace v8;

namespace wrt {
namespace service {

static int aulHandler(aul_type type, bundle *bd, void *data){
    TizenAppFW *appfw = static_cast<TizenAppFW*>(data);
    if(appfw == NULL)
        return 0;

    switch(type){
        case AUL_START:
        {
            int len;
            char *encoded_bundle;
            bundle_encode(bd, (bundle_raw**)&encoded_bundle, &len);
            appfw->OnService(encoded_bundle);
            free(encoded_bundle);
            break;
        }
        case AUL_TERMINATE:
            appfw->OnTerminate();
            break;
        default:
            LOGW("Unhandled aul event. type=%d", type);
            break;
    }
    return 0;
}

TizenAppFW& TizenAppFW::GetInstance(){
    static TizenAppFW instance;
    return instance;
}

TizenAppFW::TizenAppFW():initialized_(false){
}
TizenAppFW::~TizenAppFW(){
}

void TizenAppFW::Init(int argc, char **argv){
    if( initialized_ )
        return;
    initialized_ = true;
    aul_launch_init(aulHandler, this);
    aul_launch_argv_handler(argc, argv);
}

void TizenAppFW::OnService( const char * bundle ){
    if( service_handler_->IsFunction()){
        Handle<String> v = String::New(bundle);
        Handle<Value> args[1] = {v};
        service_handler_->Call(service_handler_, 1, args);
    }
}

void TizenAppFW::OnTerminate(){
    if( terminate_handler_->IsFunction())
        terminate_handler_->Call(terminate_handler_, 0, NULL);
}

void TizenAppFW::set_service_handler(Handle<Function> handler){
    service_handler_.Dispose();
    service_handler_ = Persistent<Function>::New(handler);
}

v8::Handle<v8::Function> TizenAppFW::service_handler(){
    return service_handler_;
}

void TizenAppFW::set_terminate_handler(Handle<Function> handler){
    terminate_handler_.Dispose();
    terminate_handler_ = Persistent<Function>::New(handler);
}

v8::Handle<v8::Function> TizenAppFW::terminate_handler(){
    return terminate_handler_;
}


static Handle<Value> onServiceGetter(Local<String> property, const AccessorInfo &info){
    HandleScope scope;
    Handle<Value> handler = TizenAppFW::GetInstance().service_handler();
    return scope.Close(handler);
}

static void onServiceSetter(Local<String> property, Local<Value> value, const AccessorInfo &info){
    HandleScope scope;
    if( value->IsFunction() )
        TizenAppFW::GetInstance().set_service_handler(Handle<Function>::Cast(value));
}

static Handle<Value> onTerminateGetter(Local<String> property, const AccessorInfo &info){
    HandleScope scope;
    Handle<Value> handler = TizenAppFW::GetInstance().terminate_handler();
    return scope.Close(handler);
}

static void onTerminateSetter(Local<String> property, Local<Value> value, const AccessorInfo &info){
    HandleScope scope;
    if( value->IsFunction() )
        TizenAppFW::GetInstance().set_terminate_handler(Handle<Function>::Cast(value));
}

static Handle<Value> appfwInit(const Arguments& args){
    HandleScope scope;
    if( args.Length() < 1 )
        return Undefined();

    if( !args[0]->IsArray() ){
        return Undefined();
    }
    Local<Array> array_args = Local<Array>::Cast(args[0]);
    int argc = array_args->Length();
    {
        char *argv[argc];
        for(int i=0; i< argc; i++){
            Local<String> arg = array_args->Get(i)->ToString();
            int nsize = arg->Utf8Length()+1;
            argv[i] = static_cast<char*>(malloc(nsize));
            memset(argv[i], 0, nsize);
            arg->WriteUtf8(argv[i]);
        }
        TizenAppFW::GetInstance().Init(argc, argv);
        for(int i=0; i<argc;i++){
            free(argv[i]);
        }
    }
	return Undefined();
}

static void init(Handle<Object> target) {
    HandleScope scope;
    TizenAppFW::GetInstance();
    target->Set(String::NewSymbol("init"), v8::FunctionTemplate::New(appfwInit)->GetFunction());
    target->SetAccessor(String::NewSymbol("onService"), onServiceGetter, onServiceSetter);
    target->SetAccessor(String::NewSymbol("onTerminate"), onTerminateGetter, onTerminateSetter);

}

NODE_MODULE(appfw, init);

} // namespace service
} // namespace wrt
