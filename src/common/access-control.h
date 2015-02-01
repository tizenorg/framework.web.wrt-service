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

#ifndef WRT_COMMON_ACCESS_CONTROL_H_
#define WRT_COMMON_ACCESS_CONTROL_H_

#include <vector>
#include <string>
#include <memory>

namespace wrt {
namespace common {

class AccessControlImpl;
class AccessControl{
public:
    static AccessControl* GetInstance();

    /**
     * Initialize Access Control module with a application id
     *
     * @remarks
     * Before All others APIs used, This API should be called.
     * Application id was set, You can not change it. 
     *
     * @param appid The application id to check
     */
    void InitAppPrivileges(const std::string& appid);

    /**
     * Check API Accessibility
     *
     * @param privileges Wanted privileges, should have a privilege at least one in these privileges
     */
    bool CheckAccessibility(const std::vector<std::string>& privileges);
    /**
     * Check API Accessibility
     *
     * @remarks
     * privileges should be end with NULL
     *
     * @param privileges Wanted privileges, should have a privilege at least one in these privileges
     */
    bool CheckAccessibility(const char** privileges);


private:
    AccessControl();
    virtual ~AccessControl();
    std::shared_ptr<AccessControlImpl> impl_;
};

} // namespace common
} // namespace wrt

#endif // WRT_COMMON_ACCESS_CONTROL_H_

