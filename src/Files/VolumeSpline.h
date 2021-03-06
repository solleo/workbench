#ifndef __VOLUME_SPLINE_H__
#define __VOLUME_SPLINE_H__

/*LICENSE_START*/
/*
 *  Copyright (C) 2014  Washington University School of Medicine
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
/*LICENSE_END*/

#include "stdint.h"
#include "CaretPointer.h"

namespace caret {
    
    class VolumeSpline
    {
        bool m_ignoredNonNumeric;
        int64_t m_dims[3];
        CaretArray<float> m_deconv;//don't do lazy deconvolution, it doesn't save much time, and takes more memory and slightly longer if you have to do the whole volume anyway
        void deconvolve(float* data, const float* backsubs, const int64_t& length);//use CaretArray so that it doesn't reallocate like a vector on copy, and the data is static once computed
        void predeconvolve(float* backsubs, const int64_t& length);//since the back substitution on the same size array uses the same coefficients, precompute them
    public:
        VolumeSpline();
        VolumeSpline(const float* frame, const int64_t framedims[3]);
        float sample(const float& i, const float& j, const float& k);
        float sample(const float ijk[3]) { return sample(ijk[0], ijk[1], ijk[2]); }
        bool ignoredNonNumeric() const { return m_ignoredNonNumeric; }
    };
    
}

#endif //__VOLUME_SPLINE_H__
