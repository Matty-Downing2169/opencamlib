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
#ifndef TRIANGLE_H
#define TRIANGLE_H

#include <vector>

#include "point.h"
#include "bbox.h"

namespace ocl
{

///
/// \brief a Triangle defined by its three vertices
///
class Triangle {
    public:
        /// default constructor
        Triangle();
        /// destructor
        virtual ~Triangle();
        /// Create a triangle with the vertices p1, p2, and p3.
        Triangle(Point p1, Point p2, Point p3);   
        
        /// return true if Triangle is sliced by a z-plane at z=zcut
        /// modify p1 and p2 so that they are intesections of the triangle edges
        /// and the plane. These vertices are used by CylCutter::edgePush()
        bool zslice_verts(Point& p1,Point& p2, const double zcut) const;
        
        
        /// Triangle string repr     
        friend std::ostream &operator<<(std::ostream &stream, const Triangle t);
        
       

        /// the three vertex Points of the Triangle
        Point p[3]; // vertices
        /// normal vector
        Point *n;    // surface normal

        /// bounding-box 
        Bbox bb;
        // these values are set from the Bbox object: (consider removing!)
        /// max x-coord
        double maxx;
        /// max y-coord
        double maxy;
        /// min x-coord
        double minx;
        /// min y-coord
        double miny;
        /// min z-coord
        double minz;
        /// max z-coord
        double maxz;

        
    protected:
        /// calculate and set Triangle normal
        void calcNormal();
        /// update bounding-box
        void calcBB();
};

} // end namespace
#endif
// end file triangle.h