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

#include <boost/foreach.hpp>

#include "conecutter.h"
#include "compositecutter.h" // for offsetCutter()
#include "numeric.h"

namespace ocl
{

ConeCutter::ConeCutter() {
    assert(0);
}

ConeCutter::ConeCutter(double d, double a, double l) {
    diameter = d;
    radius = d/2.0;
    angle = a;
    length = radius/tan(angle) + l;
    center_height = radius/tan(angle);
    xy_normal_length = radius;
    normal_length = 0.0;
}

double ConeCutter::height(double r) const {
    assert( tan(angle) > 0.0 ); // guard against division by zero
    return r/tan(angle);
}

double ConeCutter::width(double h) const {
    return (h<center_height) ? h*tan(angle) : radius ;
}

// ?? Ball-Cone-Bull ??
MillingCutter* ConeCutter::offsetCutter(double d) const {
    return new BallConeCutter(2*d,  diameter+2*d, angle) ;
}

// because this checks for contact with both the tip and the circular edge it is hard to move to the base-class
// we either hit the tip, when the slope of the plane is smaller than angle
// or when the slope is steep, the circular edge between the cone and the cylindrical shaft
bool ConeCutter::facetDrop(CLPoint &cl, const Triangle &t) const {
    bool result = false;
    Point normal = t.upNormal(); // facet surface normal    
    if ( isZero_tol( normal.z ) )  // vertical surface
        return false;  //can't drop against vertical surface
    
    if ( (isZero_tol(normal.x)) && (isZero_tol(normal.y)) ) {  // horizontal plane special case
        CCPoint cc_tmp( cl.x, cl.y, t.p[0].z, FACET_TIP );  // so any vertex is at the correct height
        return cl.liftZ_if_inFacet(cc_tmp.z, cc_tmp, t);
    } else {
        // define plane containing facet
        // a*x + b*y + c*z + d = 0, so
        // d = -a*x - b*y - c*z, where  (a,b,c) = surface normal
        double a = normal.x;
        double b = normal.y;
        double c = normal.z;
        double d = - normal.dot(t.p[0]); 
        normal.xyNormalize(); // make xy length of normal == 1.0
        // cylindrical contact point case
        // find the xy-coordinates of the cc-point
        CCPoint cyl_cc_tmp =  cl - radius*normal;
        cyl_cc_tmp.z = (1.0/c)*(-d-a*cyl_cc_tmp.x-b*cyl_cc_tmp.y);
        double cyl_cl_z = cyl_cc_tmp.z - length; // tip positioned here
        cyl_cc_tmp.type = FACET_CYL;
        
        // tip contact with facet
        CCPoint tip_cc_tmp(cl.x,cl.y,0.0);
        tip_cc_tmp.z = (1.0/c)*(-d-a*tip_cc_tmp.x-b*tip_cc_tmp.y);
        double tip_cl_z = tip_cc_tmp.z;
        tip_cc_tmp.type = FACET_TIP;
              
        result = result || cl.liftZ_if_inFacet( tip_cl_z, tip_cc_tmp, t);
        result = result || cl.liftZ_if_inFacet( cyl_cl_z, cyl_cc_tmp, t);
        return result; 
    }
}

// cone sliced with vertical plane results in a hyperbola as the intersection curve
// find point where hyperbola and line slopes match
CC_CLZ_Pair ConeCutter::singleEdgeDropCanonical( const Point& u1, const Point& u2) const {
    double d = u1.y;
    double m = (u2.z-u1.z) / (u2.x-u1.x); // slope of edge
    // the outermost point on the cutter is at   xu = sqrt( R^2 - d^2 )
    double xu = sqrt( square(radius) - square(u1.y) );                  assert( xu <= radius );
    // max slope at xu is mu = (L/(R-R2)) * xu /(sqrt( xu^2 + d^2 ))
    double mu = (center_height/radius ) * xu / sqrt( square(xu) + square(d) ) ;
    bool hyperbola_case = (fabs(m) <= fabs(mu));
    // find contact point where slopes match, there are two cases:
    // 1) if abs(m) <= abs(mu)  we contact the curve at xp = sign(m) * sqrt( R^2 m^2 d^2 / (h^2 - R^2 m^2) )
    // 2) if abs(m) > abs(mu) there is contact with the circular edge at +/- xu
    double ccu;
    if ( hyperbola_case ) { 
        ccu = sign(m) * sqrt( square(radius)*square(m)*square(d) / (square(length) -square(radius)*square(m) ) );
    } else { 
        ccu = sign(m)*xu;
    } 
    Point cc_tmp( ccu, d, 0.0); // cc-point in the XY plane
    cc_tmp.z_projectOntoEdge(u1,u2);
    double cl_z;
    if ( hyperbola_case ) {  // 1) zc = zp - Lc + (R - sqrt(xp^2 + d^2)) / tan(beta2)
        cl_z = cc_tmp.z - center_height + (radius-sqrt(square(ccu) + square(d)))/ tan(angle);
    } else {  // 2) zc = zp - Lc
        cl_z = cc_tmp.z - center_height; // case where we hit the edge of the cone
    } 
    return CC_CLZ_Pair( ccu , cl_z);
}

// cone is pushed along Fiber f into contact with edge p1-p2
bool ConeCutter::generalEdgePush(const Fiber& f, Interval& i,  const Point& p1, const Point& p2) const {
    bool result = false;
    
    // idea: as the ITO-cone slides along the edge it will pierce a z-plane at the height of the fiber
    // the shaped of the pierced area is either a circle if the edge is steep
    // or a 'half-circle' + cone shape if the edge is shallow
    // we can now intersect this 2D shape with the fiber and get the CL-points.
    // how to get the CC-point? (point on edge closest to z-axis of cutter?)
    
    // line: p1+t*(p2-p1) = zheight
    // => t = (zheight - p1)/ (p2-p1)
    if ( isZero_tol(p2.z-p1.z) )
        return result;
        
    assert( (p2.z-p1.z) != 0.0 );
    // this is where the ITO cone pierces the plane
    double t0 = (f.p1.z - p1.z) / (p2.z-p1.z);
    Point tp0 = p1 + t0*(p2-p1);
    // this is where the ITO cone exits the plane
    double t1 = (f.p1.z+center_height - p1.z) / (p2.z-p1.z);
    Point tp1 = p1 + t1*(p2-p1);
    //std::cout << "(t0, t1) (" << t0 << " , " << t1 << ") \n";
    double L = (tp1-tp0).xyNorm();
    
    if ( L <= radius ) {
        // this is where the ITO-slice is a circle
        // find intersection points, if any, between the fiber and the circle
        // fiber is f.p1 - f.p2
        // circle is centered at tp1 and radius
        double d = tp1.xyDistanceToLine(f.p1, f.p2);
        if ( d <= radius ) {
            // we know there is an intersection point.
            // http://mathworld.wolfram.com/Circle-LineIntersection.html
            double dx = f.p2.x - f.p1.x;
            double dy = f.p2.y - f.p1.y;
            double dr = sqrt( square(dx) + square(dy) );
            double det = f.p1.x * f.p2.y - f.p2.x * f.p1.y;
            
            // intersection given by:
            //  x = det*dy +/- sign(dy) * dx * sqrt( r^2 dr^2 - det^2 )   / dr^2
            //  y = -det*dx +/- abs(dy) * dx * sqrt( r^2 dr^2 - det^2 )   / dr^2
            
            double discr = square(radius) * square(dr) - square(det);
            assert( discr > 0.0 ); // this means we have an intersection
            if ( discr == 0.0 ) {
                // tangent case
                //  x = det*dy/ dr^2
                //  y = -det*dx / dr^2
            } else {
                // two intersection points
                
            }
            
            // circle ( cx + r* cosu, cy + r * sinu )   with cosu^2+sinu^2=1 => cosu = +/- sqrt( 1-sinu^2 ) 
            // fiber p1 + t*(p2-p1)
            //
            //  r*cosu = p1.x + t*(p2.x-p1.x)
            //  r*sinu = p1.x + t*(p2.x-p1.x)
        }
        return result;
    } else {
        // ITO-slice is cone + half-circle
        
        // find the tangent points as intersections between circle(diameter) and circle(L/2)
        
        return result;
    }

    //return result;
}



std::string ConeCutter::str() const {
    std::ostringstream o;
    o << *this;
    return o.str();
}

std::ostream& operator<<(std::ostream &stream, ConeCutter c) {
  stream << "ConeCutter (d=" << c.diameter << ", angle=" << c.angle << ", L=" << c.length << ")";
  return stream;
}

} // end namespace
// end file conecutter.cpp
