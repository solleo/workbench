#ifndef __DISPLAY_PROPERTIES_SURFACE_H_
#define __DISPLAY_PROPERTIES_SURFACE_H_

/*LICENSE_START*/
/* 
 *  Copyright 1995-2011 Washington University School of Medicine 
 * 
 *  http://brainmap.wustl.edu 
 * 
 *  This file is part of CARET. 
 * 
 *  CARET is free software; you can redistribute it and/or modify 
 *  it under the terms of the GNU General Public License as published by 
 *  the Free Software Foundation; either version 2 of the License, or 
 *  (at your option) any later version. 
 * 
 *  CARET is distributed in the hope that it will be useful, 
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of 
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 *  GNU General Public License for more details. 
 * 
 *  You should have received a copy of the GNU General Public License 
 *  along with CARET; if not, write to the Free Software 
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
 * 
 */ 

#include "DisplayProperties.h"
#include "SurfaceDrawingTypeEnum.h"

namespace caret {

    class Surface;
    
    class DisplayPropertiesSurface : public DisplayProperties {
        
    public:
        DisplayPropertiesSurface();
        
        virtual ~DisplayPropertiesSurface();
        
        void reset();
        
        void update();
        
        virtual void copyDisplayProperties(const int32_t sourceTabIndex,
                                           const int32_t targetTabIndex);
        
        virtual SceneClass* saveToScene(const SceneAttributes* sceneAttributes,
                                        const AString& instanceName);
        
        virtual void restoreFromScene(const SceneAttributes* sceneAttributes,
                                      const SceneClass* sceneClass);
        
        SurfaceDrawingTypeEnum::Enum getSurfaceDrawingType() const;
        
        void setSurfaceDrawingType(const SurfaceDrawingTypeEnum::Enum surfaceDrawingType);

        float getNodeSize() const;
        
        void setNodeSize(const float nodeSize);
        
        float getLinkSize() const;
        
        void setLinkSize(const float linkSize);
        
        bool isDisplayNormalVectors() const;
        
        void setDisplayNormalVectors(const bool displayNormalVectors);
        
        float getOpacity() const;
        
        void setOpacity(const float opacity);
        
    private:
        DisplayPropertiesSurface(const DisplayPropertiesSurface&);

        DisplayPropertiesSurface& operator=(const DisplayPropertiesSurface&);
        
        float m_nodeSize;
        
        float m_linkSize;
        
        bool m_displayNormalVectors;
        
        SurfaceDrawingTypeEnum::Enum m_surfaceDrawingType;
        
        float m_opacity;
    };
    
#ifdef __DISPLAY_PROPERTIES_SURFACE_DECLARE__
#endif // __DISPLAY_PROPERTIES_SURFACE_DECLARE__

} // namespace
#endif  //__DISPLAY_PROPERTIES_SURFACE_H_
