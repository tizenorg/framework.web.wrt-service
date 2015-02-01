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
#include <sqlite3.h>
#include <dlog.h>
#include <node.h>
#include <v8.h>

#include <privilege-control.h>
#include <access-control.h>



#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "WRT_SERVICE"

using namespace node;
using namespace v8;

namespace {
    const char *kWrtDBPath = "/opt/dbspace/.wrt.db";
}

namespace wrt {
namespace service {

static std::string GetStartScript(const std::string &appid){
    std::string value;
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;

    int ret = 0;
    ret = sqlite3_open(kWrtDBPath, &db);
    if( ret ){
        return value;
    }

    const char * query = "select src from WidgetStartFile where app_id = "
                 "(select app_id from WidgetInfo where tizen_appid = ?)"
                 " order by start_file_id asc limit 1";

    ret = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
    ret |= sqlite3_bind_text(stmt, 1, appid.c_str(), -1, SQLITE_TRANSIENT);

    if( ret )
        goto error;

    if( sqlite3_step(stmt) == SQLITE_ROW ){
       char startfile[1024] = {0,};
       strncpy(startfile, (char*)sqlite3_column_text(stmt, 0), 1023);
       value = startfile;
    }

error:
    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return value;
}

static void InitAce(const std::string & appid){
    wrt::common::AccessControl::GetInstance()->InitAppPrivileges(appid);
}

static void SetPrivilege(const std::string& pkgid, const std::string& path){
    SECURE_LOGD("Set privilege : %s(%s)", pkgid.c_str(), path.c_str());
    int ret = perm_app_set_privilege(pkgid.c_str(), "wgt", path.c_str());
    if (ret != PC_OPERATION_SUCCESS) {
        LOGE("error perm_app_set_privilege : (%d)", ret);
    }
}

static Handle<Value> initAce(const Arguments& args){
    HandleScope scope;
    if( args.Length() < 1 )
        return Undefined();

    std::string appid(*String::Utf8Value(args[0]->ToString()));
    InitAce(appid);

    return Undefined();
}

static Handle<Value> getStartScript(const Arguments& args){
    HandleScope scope;
    if( args.Length() < 1 )
        return Undefined();

    std::string appid(*String::Utf8Value(args[0]->ToString()));
    std::string start_script = GetStartScript(appid);

    return String::New(start_script.c_str());
}

static Handle<Value> setPrivilege(const Arguments& args){
    HandleScope scope;
    if( args.Length() < 2 ){
        LOGE("No enough arguments");
        return Undefined();
    }
    std::string pkgid(*String::Utf8Value(args[0]->ToString()));
    std::string path(*String::Utf8Value(args[1]->ToString()));
    SetPrivilege(pkgid, path);

    return Undefined();
}

static void init(Handle<Object> target) {
    HandleScope scope;
    target->Set(String::NewSymbol("getStartScript"), v8::FunctionTemplate::New(getStartScript)->GetFunction());
    target->Set(String::NewSymbol("initAce"), v8::FunctionTemplate::New(initAce)->GetFunction());
    target->Set(String::NewSymbol("setPrivilege"), v8::FunctionTemplate::New(setPrivilege)->GetFunction());
}

NODE_MODULE(serviceutil, init);

} // namespace service
} // namespace wrt

