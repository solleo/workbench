#ifndef __CLASS_AND_NAME_HIERARCHY_VIEW_CONTROLLER__H_
#define __CLASS_AND_NAME_HIERARCHY_VIEW_CONTROLLER__H_

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

#include <set>
#include <vector>

#include <QWidget>

#include "DataFileTypeEnum.h"
#include "DisplayGroupEnum.h"

class QTreeWidgetItem;
class QVBoxLayout;

namespace caret {

    class BorderFile;
    class FociFile;
    class LabelFile;
    class GroupAndNameHierarchyModel;
    class GroupAndNameHierarchyTreeWidgetItem;
    class WuQTreeWidget;
    
    class GroupAndNameHierarchyViewController : public QWidget {
        
        Q_OBJECT

    public:
        GroupAndNameHierarchyViewController(const int32_t browserWindowIndex,
                                            QWidget* parent = 0);
        
        virtual ~GroupAndNameHierarchyViewController();
        
        void updateContents(std::vector<BorderFile*> borderFiles,
                            const DisplayGroupEnum::Enum displayGroup);
        
        void updateContents(std::vector<FociFile*> fociFiles,
                            const DisplayGroupEnum::Enum displayGroup);
        
        void updateContents(std::vector<LabelFile*> labelFiles,
                            const DisplayGroupEnum::Enum displayGroup);
        
    private slots:
        void allOnPushButtonClicked();
        
        void allOffPushButtonClicked();
        
        void itemWasCollapsed(QTreeWidgetItem* item);
        
        void itemWasExpanded(QTreeWidgetItem* item);

        void itemWasChanged(QTreeWidgetItem* item,
                            int column);

    private:
        GroupAndNameHierarchyViewController(const GroupAndNameHierarchyViewController&);

        GroupAndNameHierarchyViewController& operator=(const GroupAndNameHierarchyViewController&);
        
        void updateContents(std::vector<GroupAndNameHierarchyModel*>& modelItems,
                            const DataFileTypeEnum::Enum dataFileType,
                            const bool selectionInvalidatesSurfaceNodeColoring);
        
        std::vector<GroupAndNameHierarchyModel*> getAllModels() const;
        
        void updateGraphics();
        
        void updateSelectedAndExpandedCheckboxes();
        
        void updateSelectedAndExpandedCheckboxesInOtherViewControllers();
        
        void createTreeWidget();
        
        QWidget* createAllOnOffControls();
        
        void setAllSelected(bool selected);
        
        DataFileTypeEnum::Enum m_dataFileType;
        
        /** Contains pointers to items managed by Qt, so do not delete content */
        std::vector<GroupAndNameHierarchyTreeWidgetItem*> m_treeWidgetItems;
        
        QVBoxLayout* m_modelTreeWidgetLayout;
        
        WuQTreeWidget* m_modelTreeWidget;
        
        int32_t m_browserWindowIndex;
        
        DisplayGroupEnum::Enum m_displayGroup;
        
        DisplayGroupEnum::Enum m_previousDisplayGroup;
        
        int32_t m_previousBrowserTabIndex;
        
        bool m_selectionInvalidatesSurfaceNodeColoring;
        
        static std::set<GroupAndNameHierarchyViewController*> s_allViewControllers;
        
    };
        
#ifdef __CLASS_AND_NAME_HIERARCHY_VIEW_CONTROLLER_DECLARE__
    std::set<GroupAndNameHierarchyViewController*> GroupAndNameHierarchyViewController::s_allViewControllers;
#endif // __CLASS_AND_NAME_HIERARCHY_VIEW_CONTROLLER_DECLARE__

} // namespace
#endif  //__CLASS_AND_NAME_HIERARCHY_VIEW_CONTROLLER__H_
