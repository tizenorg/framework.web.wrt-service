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
#include <sqlite3.h>
#include <string>
#include <vector>

#ifdef USE_SECURITY_CLIENT
#include <unistd.h>
#include <sys/types.h>
#include <sstream>
#include <ace_api_client.h>
#endif

#include "access-control.h"

using namespace std;

namespace {
    const char *kWrtDBPath = "/opt/dbspace/.wrt.db";
}

namespace wrt {
namespace common {

#ifndef USE_SECURITY_CLIENT
///////////////////////////////////////////////////////////////////
// use wrt db
class AccessControlImpl {
public:
    AccessControlImpl():initialized_(false){
    }
    virtual ~AccessControlImpl();
    void InitAppPrivileges(const std::string& appid);
    bool CheckAccessibility(const std::vector<std::string>& privileges);
    bool CheckAccessibility(const char** privileges);

private:
    bool initialized_;
    std::vector<std::string> granted_privileges_;
};

AccessControlImpl::~AccessControlImpl(){
}
void AccessControlImpl::InitAppPrivileges(const string & appid){
    if(initialized_)
        return;
    initialized_ = true;
    granted_privileges_.clear();

    int ret = 0;
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;

    ret = sqlite3_open(kWrtDBPath, &db);
    if( ret ){
        initialized_ = false;
        return;
    }

    const char * query = "select name from WidgetFeature where app_id = "
                 "(select app_id from WidgetInfo where tizen_appid = ?)"
                 " and rejected = 0";


    ret = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
    ret |= sqlite3_bind_text(stmt, 1, appid.c_str(), -1, SQLITE_TRANSIENT);

    if( ret ){
        initialized_ = false;
        goto error;
    }

    while( sqlite3_step(stmt) == SQLITE_ROW ){
        char privilege[1024] = {0,};
        strncpy(privilege, (const char*)sqlite3_column_text(stmt, 0), 1023);
        granted_privileges_.push_back(privilege);
    }

error:
    sqlite3_finalize(stmt);
    sqlite3_close(db);

}

bool AccessControlImpl::CheckAccessibility(const std::vector<std::string>& privileges){
    if(!initialized_)
        return false;
    bool found = false;
    std::vector<std::string>::iterator itr = granted_privileges_.begin();
    while( !found && itr != granted_privileges_.end() ){
        for( int i = 0; i< privileges.size(); i++){
            if( privileges[i] == *itr ){
                found = true;
                break;
            }
        }
        ++itr;
    }
    return found;
}

bool AccessControlImpl::CheckAccessibility(const char** privileges){
    if(!initialized_)
        return false;
    bool found = false;


    std::vector<std::string>::iterator itr = granted_privileges_.begin();
    while( !found && itr != granted_privileges_.end() ){
        for( int i = 0; privileges[i] != NULL ; i++ ){
            if( privileges[i] == *itr ){
                found = true;
                break;
            }
        }
        ++itr;
    }
    return found;
}


#else
////////////////////////////////////////////////////////////////////////
// Use security client

static ace_return_t allwaysDeny(
        ace_popup_t popup_type,
        const ace_resource_t resource_name,
        const ace_session_id_t session_id,
        const ace_param_list_t* param_list,
        ace_widget_handle_t handle,
        ace_bool_t* validation_result){
    if( validation_result)
        *validation_result = ACE_TRUE;
    return ACE_OK;
}

class AccessControlImpl {
public:
    AccessControlImpl():initialized_(false),widget_id_(-1){
    }
    virtual ~AccessControlImpl();
    void InitAppPrivileges(const std::string& appid);
    bool CheckAccessibility(const std::vector<std::string>& privileges);
    bool CheckAccessibility(const char** privileges);

private:
    bool initialized_;
    int widget_id_;
    std::string session_id_;
};

AccessControlImpl::~AccessControlImpl(){
    if( initialized_ )
        ace_client_shutdown();
}
void AccessControlImpl::InitAppPrivileges(const string & appid){
    if(initialized_)
        return;
    initialized_ = true;
    ace_client_initialize(allwaysDeny);
    stringstream ss;
    ss << appid << getpid();
    session_id_ = ss.str();

    int ret = 0;
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;

    ret = sqlite3_open(WRT_DB_PATH, &db);
    if( ret ){
        initialized_ = false;
        return;
    }

    const char * query = "select app_id from WidgetInfo where tizen_appid = ?";


    ret = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
    ret |= sqlite3_bind_text(stmt, 1, appid.c_str(), -1, SQLITE_TRANSIENT);

    if( ret ){
        initialized_ = false;
        goto error;
    }

    if( sqlite3_step(stmt) == SQLITE_ROW ){
        widget_id_ = sqlite3_column_int(stmt, 0);
    }else
        initialized_ = false;

error:
    sqlite3_finalize(stmt);
    sqlite3_close(db);

}

bool AccessControlImpl::CheckAccessibility( const std::vector<std::string>& privileges){
    if(!initialized_)
        return false;
    const char** privilegeTable;
    int count = privileges.size();
    privilegeTable = new const char*[count];
    for( int i = 0 ; i < count; i++ ){
        privilegeTable[i] = privileges[i].c_str();
    }

    bool result = CheckAccessibility(privilegeTable, count);
    delete privilegeTable;
    return result;
}

bool AccessControlImpl::CheckAccessibility(const char** privileges){
    if(!initialized_)
        return false;
    ace_request_t request;
    ace_check_result_t result;
    request.session_id = (char*)session_id_.c_str();
    request.widget_handle = widget_id_;
    request.feature_list = {0,0};
    request.dev_cap_list = {0,0};
    int count = 0;
    const char** check = privileges;
    while( check++ )
        count++;

    request.feature_list.count = count;
    request.feature_list.items = const_cast<char**>(privileges);

    ace_check_access_ex(&request, &result);
    return result == ACE_ACCESS_GRANTED;
}
#endif


AccessControl* AccessControl::GetInstance(){
    static AccessControl instance;
    return &instance;
}

AccessControl::AccessControl():impl_(new AccessControlImpl()){
}
AccessControl::~AccessControl(){
}

void AccessControl::InitAppPrivileges(const string & appid){
    impl_->InitAppPrivileges(appid);
}
bool AccessControl::CheckAccessibility(const std::vector<std::string>& privileges){
    return impl_->CheckAccessibility(privileges);
}

bool AccessControl::CheckAccessibility(const char** privileges){
    return impl_->CheckAccessibility(privileges);
}

} // namespace common
} // namespace wrt

