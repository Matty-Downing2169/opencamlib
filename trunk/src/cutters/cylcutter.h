/*  $Id$
 * 
 *  Copyright 2010 Anders Wallin (anders.e.e.wallin "at" gmail.com)
 *  
 *  This file is part of OpenCAMlib.
 *
 *  OpenCAMlib is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  OpenCAMlib is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with OpenCAMlib.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef CYL_CUTTER_H
#define CYL_CUTTER_H

#include <iostream>
#include <string>
#include <vector>

#include "millingcutter.h"

namespace ocl
{

///
/// \brief Cylindrical MillingCutter (flat-endmill)
///
/// defined by one parameter, the cutter diameter
class CylCutter : public MillingCutter {
    public:
        CylCutter();
        /// create CylCutter with diameter d and length l
        explicit CylCutter(const double d, const double l);

        MillingCutter* offsetCutter(const double d) const;
        
        CC_CLZ_Pair  singleEdgeContact(const Point& u1, const Point& u2) const;
        
        bool vertexPush(const Fiber& f, Interval& i, const Triangle& t) const;
        bool edgePush(const Fiber& f, Interval& i, const Triangle& t) const;
        
        friend std::ostream& operator<<(std::ostream &stream, CylCutter c);
        std::string str() const;
    protected:
        double height(const double r) const;
        double width(const double h) const; 
};

} // end namespace
#endif
