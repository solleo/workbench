
/*LICENSE_START*/
/*
 * Copyright 2013 Washington University,
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

#include <cmath>

#define __BRAIN_OPEN_G_L_SHAPE_CYLINDER_DECLARE__
#include "BrainOpenGLShapeCylinder.h"
#undef __BRAIN_OPEN_G_L_SHAPE_CYLINDER_DECLARE__

#include "CaretAssert.h"
#include "MathFunctions.h"

using namespace caret;
    
/**
 * \class caret::BrainOpenGLShapeCylinder 
 * \brief Creates a cylinder shape for use with OpenGL.
 * \ingroup Brain
 */

/**
 * Constructor.
 */
BrainOpenGLShapeCylinder::BrainOpenGLShapeCylinder(const int32_t numberOfSides)
: BrainOpenGLShape(),
m_numberOfSides(numberOfSides)
{
    m_displayList    = 0;
    
    m_coordinatesBufferID = 0;
    m_coordinatesRgbaByteBufferID = 0;
    m_coordinatesRgbaFloatBufferID = 0;
    
    m_normalsBufferID = 0;
    m_triangleStripBufferID = 0;
    
    m_isApplyColoring = true;
}

/**
 * Destructor.
 */
BrainOpenGLShapeCylinder::~BrainOpenGLShapeCylinder()
{
}

void
BrainOpenGLShapeCylinder::setupShape(const BrainOpenGL::DrawMode drawMode)
{
//    bool debugFlag = false;
    
    
    /*
     * Setup step size based upon number of points around circle
     */
    const float step = (2.0 * M_PI) / m_numberOfSides;
    
    
    /*
     * Z-coordinates at bottom and top of cylinder
     */
    const float zTop =     1.0;
    const float zBottom =  0.0;
    
    std::vector<GLuint> topTriangleStripVertices;
    std::vector<GLuint> bottomTriangleStripVertices;
    std::vector<GLuint> sideTriangleStripVertices;
    
    /*
     * Counts vertices
     */
    GLuint vertexCounter = 0;
    
//    const float topCenterNormalX = 0.0;
//    const float topCenterNormalY = 0.0;
//    const float topCenterNormalZ = 1.0;
//    
//    const float topCenterX = 0.0;
//    const float topCenterY = 0.0;
//    const float topCenterZ = 1.0;
//    
//    const float bottomCenterNormalX =  0.0;
//    const float bottomCenterNormalY =  0.0;
//    const float bottomCenterNormalZ = -1.0;
//    
//    const float bottomCenterX =  0.0;
//    const float bottomCenterY =  0.0;
//    const float bottomCenterZ = -1.0;
    
//    /*
//     * Center of top
//     */
//    m_coordinates.push_back(0.0);
//    m_coordinates.push_back(0.0);
//    m_coordinates.push_back(1.0);
//    m_normals.push_back(topCenterNormalX);
//    m_normals.push_back(topCenterNormalY);
//    m_normals.push_back(topCenterNormalZ);
//    m_normals.push_back(vertexCounter);
//    vertexCounter++;
//    
//    /*
//     * Center of bottom
//     */
//    m_coordinates.push_back(0.0);
//    m_coordinates.push_back(0.0);
//    m_coordinates.push_back(-1.0);
//    m_normals.push_back(bottomCenterNormalX);
//    m_normals.push_back(bottomCenterNormalY);
//    m_normals.push_back(bottomCenterNormalZ);
//    m_normals.push_back(vertexCounter);
//    vertexCounter++;
    
    const GLuint firstSideVertex = vertexCounter;
    
    /*
     * Generate points around cylinder
     */
    const float radius = 0.5;
    for (int32_t i = 0; i < m_numberOfSides; i++) {
        const float t = step * i;
        
        const float x = radius * std::cos(t);
        const float y = radius * std::sin(t);

        /*
         * Top of slice
         */
        m_coordinates.push_back(x);
        m_coordinates.push_back(y);
        m_coordinates.push_back(zTop);
        m_normals.push_back(x);
        m_normals.push_back(y);
        m_normals.push_back(0.0);
        sideTriangleStripVertices.push_back(vertexCounter);
        vertexCounter++;
        
        
//        m_normals.push_back(topCenterNormalX);
//        m_normals.push_back(topCenterNormalY);
//        m_normals.push_back(topCenterNormalZ);
//        m_topTriangleFan.push_back(vertexCounter);
        
        m_coordinates.push_back(x);
        m_coordinates.push_back(y);
        m_coordinates.push_back(zBottom);
        m_normals.push_back(x);
        m_normals.push_back(y);
        m_normals.push_back(0.0);
        sideTriangleStripVertices.push_back(vertexCounter);
        vertexCounter++;
        
//        m_bottomNormals.push_back(bottomCenterNormalX);
//        m_bottomNormals.push_back(bottomCenterNormalY);
//        m_bottomNormals.push_back(bottomCenterNormalZ);
//        m_bottomTriangleFan.push_back(vertexCounter);
//        vertexCounter++;
    }

    /*
     * Finish cylinder by specifying first two coordinates again
     */
    const GLuint firstVertexIndex = firstSideVertex * 3;
    m_coordinates.push_back(m_coordinates[firstVertexIndex]);
    m_coordinates.push_back(m_coordinates[firstVertexIndex+1]);
    m_coordinates.push_back(m_coordinates[firstVertexIndex+2]);
    m_normals.push_back(m_normals[firstVertexIndex]);
    m_normals.push_back(m_normals[firstVertexIndex+1]);
    m_normals.push_back(m_normals[firstVertexIndex+2]);
    sideTriangleStripVertices.push_back(vertexCounter);
    vertexCounter++;
    
    m_coordinates.push_back(m_coordinates[firstVertexIndex+3]);
    m_coordinates.push_back(m_coordinates[firstVertexIndex+4]);
    m_coordinates.push_back(m_coordinates[firstVertexIndex+5]);
    m_normals.push_back(m_normals[firstVertexIndex+3]);
    m_normals.push_back(m_normals[firstVertexIndex+4]);
    m_normals.push_back(m_normals[firstVertexIndex+5]);
    sideTriangleStripVertices.push_back(vertexCounter);
    vertexCounter++;
    
    CaretAssert((vertexCounter * 3) == static_cast<GLuint>(m_coordinates.size()));
    CaretAssert((vertexCounter * 3) == static_cast<GLuint>(m_normals.size()));
    
    m_triangleStrip = sideTriangleStripVertices;
    
//    /*
//     * Finish top and bottom
//     * Note bottom vertices need to be reverse so that 
//     * that the vertices are counter clockwise when pointing down.
//     */
//    m_topNormals.push_back(m_topNormals[0]);
//    m_topNormals.push_back(m_topNormals[1]);
//    m_topNormals.push_back(m_topNormals[2]);
//    m_topTriangleFan.push_back(firstSideVertex);
//
//    m_bottomNormals.push_back(m_bottomNormals[0]);
//    m_bottomNormals.push_back(m_bottomNormals[1]);
//    m_bottomNormals.push_back(m_bottomNormals[2]);
//    m_bottomTriangleFan.push_back(firstSideVertex + 1);
//    std::reverse(m_bottomTriangleFan.begin(),
//                 m_bottomTriangleFan.end());
//
//    CaretAssert(m_topNormals.size() == (m_topTriangleFan.size() * 3));
//    CaretAssert(m_sidesNormals.size() == ((m_sidesTriangleStrip.size() - 2) * 3));
//    CaretAssert(m_topNormals.size() == (m_topTriangleFan.size() * 3));

    /*
     * Create storage for colors
     */
    m_rgbaByte.resize(vertexCounter * 4, 0);
    m_rgbaFloat.resize(vertexCounter * 4, 0.0);
    for (GLuint i = 0; i < vertexCounter; i++) {
        const int32_t i4 = i * 4;

        m_rgbaFloat[i4]   = 0.0;
        m_rgbaFloat[i4+1] = 0.0;
        m_rgbaFloat[i4+2] = 0.0;
        m_rgbaFloat[i4+3] = 1.0;
        
        m_rgbaByte[i4]   = 0;
        m_rgbaByte[i4+1] = 0;
        m_rgbaByte[i4+2] = 0;
        m_rgbaByte[i4+3] = 255;
    }
        
    switch (drawMode) {
        case BrainOpenGL::DRAW_MODE_DISPLAY_LISTS:
        {
            m_displayList = createDisplayList();
            
            if (m_displayList > 0) {
                glNewList(m_displayList,
                          GL_COMPILE);
                uint8_t rgbaUnused[4] = { 0.0, 0.0, 0.0, 0.0 };
                m_isApplyColoring = false;
                drawShape(BrainOpenGL::DRAW_MODE_IMMEDIATE,
                          rgbaUnused);
                m_isApplyColoring = true;
                glEndList();
            }
        }
            break;
        case BrainOpenGL::DRAW_MODE_IMMEDIATE:
        {
            /* nothing to do for this case */
        }
            break;
        case BrainOpenGL::DRAW_MODE_INVALID:
        {
            CaretAssert(0);
        }
            break;
        case BrainOpenGL::DRAW_MODE_VERTEX_BUFFERS:
#ifdef BRAIN_OPENGL_INFO_SUPPORTS_VERTEX_BUFFERS
            if (BrainOpenGL::isVertexBuffersSupported()) {
                /*
                 * Put coordinates into its buffer.
                 */
                m_coordinatesBufferID = createBufferID();
                glBindBuffer(GL_ARRAY_BUFFER,
                             m_coordinatesBufferID);
                glBufferData(GL_ARRAY_BUFFER,
                             m_coordinates.size() * sizeof(GLfloat),
                             &m_coordinates[0],
                             GL_STATIC_DRAW);
                
                /*
                 * Put BYTE colors into its buffer
                 */
                m_coordinatesRgbaByteBufferID = createBufferID();
                glBindBuffer(GL_ARRAY_BUFFER,
                             m_coordinatesRgbaByteBufferID);
                glBufferData(GL_ARRAY_BUFFER,
                             m_rgbaByte.size() * sizeof(GLubyte),
                             &m_rgbaByte[0],
                             GL_DYNAMIC_DRAW);
                
                /*
                 * Put FLOAT colors into its buffer
                 */
                m_coordinatesRgbaFloatBufferID = createBufferID();
                glBindBuffer(GL_ARRAY_BUFFER,
                             m_coordinatesRgbaFloatBufferID);
                glBufferData(GL_ARRAY_BUFFER,
                             m_rgbaFloat.size() * sizeof(GLfloat),
                             &m_rgbaFloat[0],
                             GL_DYNAMIC_DRAW);
                
                
                /*
                 * Put side normals into its buffer.
                 */
                m_normalsBufferID = createBufferID();
                glBindBuffer(GL_ARRAY_BUFFER,
                             m_normalsBufferID);
                glBufferData(GL_ARRAY_BUFFER,
                             m_normals.size() * sizeof(GLfloat),
                             &m_normals[0],
                             GL_STATIC_DRAW);
                
                /*
                 * Put sides triangle strip for sides into its buffer.
                 */
                m_triangleStripBufferID = createBufferID();
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,
                             m_triangleStripBufferID);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                             m_triangleStrip.size() * sizeof(GLuint),
                             &m_triangleStrip[0],
                             GL_STATIC_DRAW);
                
                /*
                 * Deselect active buffer.
                 */
                glBindBuffer(GL_ARRAY_BUFFER,
                             0);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,
                             0);
            }
#endif // BRAIN_OPENGL_INFO_SUPPORTS_VERTEX_BUFFERS
            break;
    }
}

/**
 * Draw the shape.
 *
 * @param drawMode
 *   How to draw the shape.
 * @param rgba
 *   RGBA coloring ranging 0.0 to 1.0
 */
void
BrainOpenGLShapeCylinder::drawShape(const BrainOpenGL::DrawMode drawMode,
                                const float rgba[4])
{
    const uint8_t rgbaByte[4] = {
        static_cast<uint8_t>(rgba[0] * 255.0),
        static_cast<uint8_t>(rgba[1] * 255.0),
        static_cast<uint8_t>(rgba[2] * 255.0),
        static_cast<uint8_t>(rgba[3] * 255.0)
    };
    
    drawShape(drawMode,
              rgbaByte);
}

/**
 * Draw the shape.
 *
 * @param drawMode
 *   How to draw the shape.
 * @param rgba
 *   RGBA coloring ranging 0 to 255.
 */
/**
 * Draw the cone.
 * @param drawMode
 *   How to draw the shape.
 */
void
BrainOpenGLShapeCylinder::drawShape(const BrainOpenGL::DrawMode drawMode,
                                    const uint8_t rgba[4])
{
    if (m_isApplyColoring) {
        glColor4ubv(rgba);
    }
    
    switch (drawMode) {
        case BrainOpenGL::DRAW_MODE_DISPLAY_LISTS:
        {
            if (m_displayList > 0) {
                glCallList(m_displayList);
            }
        }
            break;
        case BrainOpenGL::DRAW_MODE_IMMEDIATE:
        {
//            /*
//             * Draw the top
//             */
//            const int32_t numTopVertices = static_cast<int32_t> (m_topTriangleFan.size());
//            glBegin(GL_TRIANGLE_FAN);
//            for (int32_t i = 0; i < numTopVertices; i++) {
//                CaretAssertVectorIndex(m_topTriangleFan, i);
//                const int32_t fanIndex = m_topTriangleFan[i];
//                const int32_t coordinateOffset = fanIndex * 3;
//                const int32_t colorOffset = fanIndex * 4;
//                const int32_t normalOffset = i * 3;
//                
//                CaretAssertVectorIndex(m_coordinates, coordinateOffset + 2);
//                CaretAssertVectorIndex(m_topNormals, normalOffset + 2);
//                CaretAssertVectorIndex(m_coordinatesRGBA, colorOffset + 3);
//                
//                glColor4fv(&m_coordinatesRGBA[colorOffset]);
//                glNormal3fv(&m_topNormals[normalOffset]);
//                glVertex3fv(&m_coordinates[coordinateOffset]);
//            }
//            glEnd();
//            
//            /*
//             * Draw the bottom
//             */
//            const int32_t numBottomVertices = static_cast<int32_t> (m_bottomTriangleFan.size());
//            glBegin(GL_TRIANGLE_FAN);
//            for (int32_t i = 0; i < numBottomVertices; i++) {
//                CaretAssertVectorIndex(m_bottomTriangleFan, i);
//                const int32_t fanIndex = m_bottomTriangleFan[i];
//                const int32_t coordinateOffset = fanIndex * 3;
//                const int32_t colorOffset = fanIndex * 4;
//                const int32_t normalOffset = i * 3;
//                
//                CaretAssertVectorIndex(m_coordinates, coordinateOffset + 2);
//                CaretAssertVectorIndex(m_bottomNormals, normalOffset + 2);
//                CaretAssertVectorIndex(m_coordinatesRGBA, colorOffset + 3);
//                
//                glColor4fv(&m_coordinatesRGBA[colorOffset]);
//                glNormal3fv(&m_bottomNormals[normalOffset]);
//                glVertex3fv(&m_coordinates[coordinateOffset]);
//            }
//            glEnd();
            
            /*
             * Draw the sides
             */
            const int32_t numSideVertices = static_cast<int32_t>(m_triangleStrip.size());
            glBegin(GL_TRIANGLE_STRIP);
            for (int32_t i = 0; i < numSideVertices; i++) {
                CaretAssertVectorIndex(m_triangleStrip, i);
                const int32_t stripIndex = m_triangleStrip[i];
                const int32_t coordinateOffset = stripIndex * 3;
                //const int32_t colorOffset = stripIndex * 4;
                const int32_t normalOffset = stripIndex * 3;
                
                CaretAssertVectorIndex(m_coordinates, coordinateOffset + 2);
                CaretAssertVectorIndex(m_normals, normalOffset + 2);
                //CaretAssertVectorIndex(m_rgba, colorOffset + 3);
                
                //glColor4ubv(&m_rgba[colorOffset]);
                glNormal3fv(&m_normals[normalOffset]);
                glVertex3fv(&m_coordinates[coordinateOffset]);
            }
            glEnd();
        }
            break;
        case BrainOpenGL::DRAW_MODE_INVALID:
        {
            CaretAssert(0);
        }
            break;
        case BrainOpenGL::DRAW_MODE_VERTEX_BUFFERS:
#ifdef BRAIN_OPENGL_INFO_SUPPORTS_VERTEX_BUFFERS
            if (BrainOpenGL::isVertexBuffersSupported()) {
                /*
                 * Enable vertices and normals for buffers
                 */
                glEnableClientState(GL_VERTEX_ARRAY);
                glEnableClientState(GL_NORMAL_ARRAY);
                glEnableClientState(GL_COLOR_ARRAY);
                
                /*
                 * Set the vertices for drawing.
                 */
                glBindBuffer(GL_ARRAY_BUFFER,
                             m_coordinatesBufferID);
                glVertexPointer(3,
                                GL_FLOAT,
                                0,
                                (GLvoid*)0);
                
                /*
                 * Set BYTE color components for drawing
                 */
                glBindBuffer(GL_ARRAY_BUFFER,
                             m_coordinatesRgbaByteBufferID);
                glColorPointer(4,
                                GL_UNSIGNED_BYTE,
                                0,
                                (GLvoid*)0);
                
                /*
                 * Set FLOAT color components for drawing
                 */
                glBindBuffer(GL_ARRAY_BUFFER,
                             m_coordinatesRgbaFloatBufferID);
                glColorPointer(4,
                               GL_FLOAT,
                               0,
                               (GLvoid*)0);
                
                /*
                 * Set the side normal vectors for drawing.
                 */
                glBindBuffer(GL_ARRAY_BUFFER,
                             m_normalsBufferID);
                glNormalPointer(GL_FLOAT,
                                0,
                                (GLvoid*)0);
                
                /*
                 * Draw the side triangle strip.
                 */
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,
                             m_triangleStripBufferID);
                glDrawElements(GL_TRIANGLE_STRIP,
                               m_triangleStrip.size(),
                               GL_UNSIGNED_INT,
                               (GLvoid*)0);
                /*
                 * Draw the bottom cap triangle fans.
                 */
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,
                             m_triangleStripBufferID);
                glDrawElements(GL_TRIANGLE_STRIP,
                               m_triangleStrip.size(),
                               GL_UNSIGNED_INT,
                               (GLvoid*)0);
                
                /*
                 * Deselect active buffer.
                 */
                glBindBuffer(GL_ARRAY_BUFFER,
                             0);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,
                             0);
                
                /*
                 * Disable vertices and normals for buffers.
                 * Otherwise, bad thing will happen.
                 */
                glDisableClientState(GL_VERTEX_ARRAY);
                glDisableClientState(GL_NORMAL_ARRAY);
                glDisableClientState(GL_COLOR_ARRAY);
            }
#endif // BRAIN_OPENGL_INFO_SUPPORTS_VERTEX_BUFFERS
            break;
    }
}