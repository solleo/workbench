#ifndef __CLASS_AND_NAME_HIERARCHY_GROUP_USER_KEY__H_
#define __CLASS_AND_NAME_HIERARCHY_GROUP_USER_KEY__H_

/*LICENSE_START*/
/*
 * Copyright 2012 Washington University,
 * All rights reserved.
 *
 * Connectome DB and Connectome Workbench are part of the integrated Connectome 
 * Informatics Platform.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *    * Neither the names of Washington University nor the
 *      names of its contributors may be used to endorse or promote products
 *      derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR  
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE.
 */
/*LICENSE_END*/


#include "GroupAndNameHierarchyGroup.h"

namespace caret {

    class GroupAndNameHierarchyGroupUserKey : public GroupAndNameHierarchyGroup {
    public:
        GroupAndNameHierarchyGroupUserKey(const AString& name,
                                         const int32_t key);
        
        ~GroupAndNameHierarchyGroupUserKey();
        
        void clear();
        
        void addNameWithKey(const AString& name,
                            const int32_t nameKey);
        
        GroupAndNameHierarchyName* getNameSelectorWithKey(const int32_t nameKey);
        
        const GroupAndNameHierarchyName* getNameSelectorWithKey(const int32_t nameKey) const;
        
        int32_t getNumberOfNamesWithCountersGreaterThanZero() const;
        
        void clearAllNameCounters();
        
        void removeNamesWithCountersEqualZero();
        
        // ADD_NEW_METHODS_HERE
        
    private:
        /** Indexes name information by name key.  Vector provides fast access by key. */
        std::map<int32_t, GroupAndNameHierarchyName*> keyToNameSelectorMap;

        GroupAndNameHierarchyGroupUserKey(const GroupAndNameHierarchyGroupUserKey&);

        GroupAndNameHierarchyGroupUserKey& operator=(const GroupAndNameHierarchyGroupUserKey&);
        
        // ADD_NEW_MEMBERS_HERE

    };
    
#ifdef __CLASS_AND_NAME_HIERARCHY_GROUP_USER_KEY_DECLARE__
    // <PLACE DECLARATIONS OF STATIC MEMBERS HERE>
#endif // __CLASS_AND_NAME_HIERARCHY_GROUP_USER_KEY_DECLARE__

} // namespace
#endif  //__CLASS_AND_NAME_HIERARCHY_GROUP_USER_KEY__H_
