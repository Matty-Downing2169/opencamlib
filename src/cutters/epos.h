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

#ifndef EPOS_H
#define EPOS_H

#include <list>
#include "point.h"

namespace ocl
{
    
class Ellipse;
///
/// \brief Epos defines a position in (s,t) coordinates on a unit-circle.
/// The (s,t) pair is used to locate points on an ellipse.
/// 
/// s^2 + t^2 = 1 should be true at all times.
class Epos {
    public:
        /// create an epos
        Epos();
        
        /// set (s,t) pair to the position corresponding to diangle
        void setD();
        
        /// s-parameter in [-1, 1]
        double s;
        /// t-parameter in [-1, 1]
        double t;
        
        /// diamond angle parameter in [0,4] (modulo 4)
        /// this models an angle [0,2pi] and maps 
        /// from the angle to an (s,t) pair using setD()
        double diangle;
        
        /// set rhs Epos (s,t) values equal to lhs Epos
        Epos &operator=(const Epos &pos);
        
        /// string repr
        std::string str() const;
        /// string repr
        friend std::ostream& operator<<(std::ostream &stream, Epos pos);
        
        /// return true if (s,t) is valid, i.e. lies on the unit circle
        /// checks s^2 + t^2 == 1  (to within tolerance) 
        bool isValid() const;
};
    
}// end namespace
#endif
// end file epos.h