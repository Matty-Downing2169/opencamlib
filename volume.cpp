/*  Copyright 2010 Anders Wallin (anders.e.e.wallin "at" gmail.com)
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

#include <iostream>
#include <stdio.h>
#include <sstream>
#include <math.h>
#include <vector>
#include <algorithm>
#include <boost/python.hpp>
#include <list>

// uncomment to disable assert() calls
// #define NDEBUG
#include <cassert>

#include "cutter.h"
#include "point.h"
#include "triangle.h"
#include "cutter.h"
#include "numeric.h"
#include "octree.h"
#include "volume.h"

//************* Sphere **************/

/// sphere at center
SphereOCTVolume::SphereOCTVolume()
{
    center = Point(2,0,0);
    radius = 3.0;
}


bool SphereOCTVolume::isInside(Point& p) const
{
    
    if ( (center-p).norm() <= radius ) {
        //std::cout << "dist to point=" << (center-p).norm() <<"\n";
        return true;
    }
    else
        return false;
}


//************* Cube **************/

/// cube at center with side length side
CubeOCTVolume::CubeOCTVolume()
{
    center = Point(0,0,0);
    side = 1.234;
}

bool CubeOCTVolume::isInside(Point& p) const
{
    bool x,y,z;
    x = ( (p.x >= (center.x-side/2)) && (p.x <= (center.x+side/2)) );
    y = ( (p.y >= (center.y-side/2)) && (p.y <= (center.y+side/2)) );
    z = ( (p.z >= (center.z-side/2)) && (p.z <= (center.z+side/2)) );
    if ( x && y && z )
        return true;
    else
        return false;
}

//************* Cylinder **************/

CylinderOCTVolume::CylinderOCTVolume()
{
    p1 = Point(0,0,0);
    p2 = Point(1,1,1);
    radius = 1.234;
}

bool CylinderOCTVolume::isInside(Point& p) const 
{
    // closest point on axis
    Point c = p.closestPoint(p1, p2);
    
    // line = p1 + t*(p2-p1)
    // t is in [0,1] for points on the line
    
    double t = (c.dot(p2-p1) - p1.dot( p2-p1)) / (p2-p1).dot(p2-p1);
    if ( (t>1.0) || (t < 0.0))
        return false;
        
    if ( (c-p).norm() <= radius)
        return true;
    else
        return false;
}


//************* CylCutterMove **************/


CylMoveOCTVolume::CylMoveOCTVolume(CylCutter& cin, Point& p1in, Point& p2in)
{
    p1 = p1in;
    p2 = p2in;
    c = cin;
}

bool CylMoveOCTVolume::isInside(Point& p) const 
{
    // CL follows line
    // line = p1 + t*(p2-p1)
    // top of cutter follows same line only c.length() higher
    
    
    // closest point on axis
    Point close = p.closestPoint(p1, p2);
    
    // line = p1 + t*(p2-p1)
    // t is in [0,1] for points on the line
    double t = (close.dot(p2-p1) - p1.dot( p2-p1)) / (p2-p1).dot(p2-p1);
    
    if ( (t>1.0) || (t < 0.0)) // check that closest point is within p1-p2 segment
        return false;
        
        
    if ( (close-p).norm() <= c.getRadius()) // check that p is within cylinder
        return true;
    else
        return false;
}

// end of file volume.cpp