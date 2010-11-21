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

#include <set>
#include <stack>

#include <boost/foreach.hpp>
#include <boost/graph/adjacency_list.hpp> 
#include <boost/graph/connected_components.hpp>
#include <boost/graph/planar_face_traversal.hpp>
#include <boost/graph/boyer_myrvold_planar_test.hpp>

#include "voronoidiagram.h"
#include "numeric.h"

namespace ocl
{

VoronoiDiagram::VoronoiDiagram() {
    far_radius = 100;
    init();
}

VoronoiDiagram::~VoronoiDiagram() {
    //
}

VoronoiVertex VoronoiDiagram::add_vertex( Point pos, VoronoiVertexType t) {
    VoronoiVertex v = boost::add_vertex(vd);
    vd[v].position = pos;
    vd[v].type = t;
    return v;
}

VoronoiEdge VoronoiDiagram::add_edge(VoronoiVertex v1, VoronoiVertex v2) {
    VoronoiEdge e;
    bool b;
    boost::tie( e , b ) = boost::add_edge( v1, v2, vd);
    return e;
}

/*
void VoronoiEdge VoronoiDiagram::add_face_and_edges( Point generator, VoronoiVertex v0, VoronoiVertex v1, VoronoiVertex v2) {
    // v0-v1-v2 are given in CCW order, thus face f is allways to the left of all edges
    VoronoiEdge e1 = add_edge( v0, v1 );   
    VoronoiEdge e2 = add_edge( v1, v2 );
    VoronoiEdge e3 = add_edge( v2, v0 ); // face, twin, next
    FaceIdx f =  faces.add_face(e1, generator, NONINCIDENT);
    vd[e1].face = f;
    vd[e2].face = f;
    vd[e3].face = f;
    vd[e1].next = e2;
    vd[e2].next = e3;
    vd[e3].next = e1;
}*/

void VoronoiDiagram::init() {
    double far_multiplier = 6;
    // add vertices
    VoronoiVertex v0     = add_vertex( Point(0,0), UNDECIDED);
    VoronoiVertex v1     = add_vertex( Point(0, far_multiplier*far_radius), OUT );
    VoronoiVertex v2     = add_vertex( Point( cos(-5*PI/6)*far_multiplier*far_radius, sin(-5*PI/6)*far_multiplier*far_radius), OUT );
    VoronoiVertex v3     = add_vertex( Point( cos(-PI/6)*far_multiplier*far_radius, sin(-PI/6)*far_multiplier*far_radius), OUT );

    // the locations of the initial generators:
    Point gen2 = Point(cos(PI/6)*3*far_radius, sin(PI/6)*3*far_radius);
    Point gen3 = Point(cos(5*PI/6)*3*far_radius, sin(5*PI/6)*3*far_radius);
    Point gen1 = Point( 0,-3*far_radius);
    // set props for center vd-point
    vd[v0].set_J( gen1, gen2, gen3 ); // this sets J2,J3,J4 and pk, so that detH(pl) can be called later
    
    // Point outer = Point(0,0,1e9); // the outer face at infinity
    
    // add face 1: v0-v1-v2
    VoronoiEdge e1 = add_edge( v0, v1 );   
    VoronoiEdge e2 = add_edge( v1, v2 );
    VoronoiEdge e3 = add_edge( v2, v0 ); // face, twin, next
    FaceIdx f1 =  faces.add_face(e2, gen3, NONINCIDENT);
    vd[e1].face = f1;
    vd[e2].face = f1;
    vd[e3].face = f1;
    vd[e1].next = e2;
    vd[e2].next = e3;
    vd[e3].next = e1;
    
    // add face 2: v0-v2-v3
    VoronoiEdge e4 = add_edge( v0, v2 );   
    VoronoiEdge e5 = add_edge( v2, v3 );
    VoronoiEdge e6 = add_edge( v3, v0 ); // face, twin, next
    FaceIdx f2 =  faces.add_face(e5, gen1, NONINCIDENT);
    vd[e4].face = f2;
    vd[e5].face = f2;
    vd[e6].face = f2;
    vd[e4].next = e5;
    vd[e5].next = e6;
    vd[e6].next = e4;
    
    // add face 3: v0-v3-v1 
    VoronoiEdge e7 = add_edge( v0, v3 );   
    VoronoiEdge e8 = add_edge( v3, v1 );
    VoronoiEdge e9 = add_edge( v1, v0 ); // face, twin, next
    FaceIdx f3 =  faces.add_face(e5, gen2, NONINCIDENT);
    vd[e7].face = f3;
    vd[e8].face = f3;
    vd[e9].face = f3;
    vd[e7].next = e8;
    vd[e8].next = e9;
    vd[e9].next = e7;
    
    // twin edges
    vd[e1].twin = e9;
    vd[e2].twin = VoronoiEdge();
    vd[e3].twin = e4;
    vd[e4].twin = e3;
    vd[e5].twin = VoronoiEdge();
    vd[e6].twin = e7;
    vd[e7].twin = e6;
    vd[e8].twin = VoronoiEdge();
    vd[e9].twin = e1;
}



void VoronoiDiagram::addVertexSite(Point p) {
    // 1) find the dd-vertex closest to p
    // 2) find the vd-face corresponding to p
    // 3) amont the vd-vertices in the found face, find the one with the smallest H
    // 4) expand the tree of to-be-deleted vertices as far as possible:
    //   4.1 tree property
    //   4.2 H<0 property
    
    // 1)
    // B1.1
    FaceIdx closest_face = find_closest_face( p );
    std::cout << " closest face is "<< closest_face << " at " << faces[closest_face].generator << "\n";
    // FIXME TODO faces.add_face( edge?, p);
    //std::cout << " added new vertex to dd: " << dd_new << "\n";

    // B1.2 find seed vertex by evaluating H on the vertices of the found face
    VoronoiVertex vd_seed = find_seed_vertex(closest_face, p);
    
    std::cout << " seed vertex is " << vd_seed << " at " << vd[vd_seed].position << "\n";
    
    // add seed to set V0
    VertexVector v0;
    v0.push_back(vd_seed);
    vd[vd_seed].type = IN;
    // find the set V0
    augment_vertex_set(v0);
    
    add_new_voronoi_vertices(v0, p);
    

    
    // generate new edges that form a loop around the region to be deleted
    // traverse the v0 tree, storing new vertices in the order we encounter them
    //VertexVector new_vd_verts = traverse_v0_tree(v0);
    //std::cout << " got " << new_vd_verts.size() << " NEW verts when traversing tree \n";
    
    // new_vd_verts now has the new vertices in the correct order.
    FaceVector incident_faces = get_incident_faces();
    
    // create new face and pass to split_face() ?
    // FaceIdx f3 =  faces.add_face(e5, p, NONINCIDENT);
    
    BOOST_FOREACH( FaceIdx f, incident_faces ) {
        split_face(f);
    }
    
    // remove vertices V0 and edges
    BOOST_FOREACH( VoronoiVertex v, v0 ) {
        boost::clear_vertex( v, vd);
    }
    BOOST_FOREACH( VoronoiVertex v, v0 ) {
        boost::remove_vertex( v, vd);
    }
}

void VoronoiDiagram::split_face(FaceIdx f) {
    VoronoiEdge startEdge = faces[f].edge;
    
}

FaceVector VoronoiDiagram::get_incident_faces() {
    FaceVector output;
    for (FaceIdx m=0;m<faces.size(); ++m) {
        if ( faces[m].type == INCIDENT )
            output.push_back(m);
    }
    return output;
}

void VoronoiDiagram::add_new_voronoi_vertices(VertexVector& v0, Point& p) {
    // generate new vertices on all edges in V0 connecting to OUT-vertices
    BOOST_FOREACH( VoronoiVertex v, v0 ) {
        assert( vd[v].type == IN );
        VoronoiOutEdgeItr edgeItr, edgeItrEnd;
        boost::tie( edgeItr, edgeItrEnd ) = boost::out_edges(v, vd);
        std::vector<VoronoiEdge> q_edges; // new vertices generated on these edges
        for ( ; edgeItr != edgeItrEnd ; ++edgeItr ) {
            VoronoiVertex adj_vertex = boost::target( *edgeItr, vd );
            if ( vd[adj_vertex].type == OUT ) 
                q_edges.push_back(*edgeItr);// adding vertices here invalidates the iterator edgeItr, so we can't do it here...
        }
        for( unsigned int m=0; m<q_edges.size(); ++m)  {  // create new vertices on edges q_edges[]
            VoronoiVertex q = boost::add_vertex( vd ) ;
            vd[q].type = NEW;
            FaceIdx left_face = vd[q_edges[m]].face;
            VoronoiEdge twin_edge = vd[q_edges[m]].twin;
            FaceIdx right_face = vd[twin_edge].face;
            Point generator1 = faces[left_face].generator;
            Point generator2 = faces[right_face].generator;
            vd[q].set_J( generator1, generator2, p); // ordering of generators shoud be CCW !?
            vd[q].set_position();
            std::cout << " added vertex "<< q << " on edge " << q_edges[m] <<" at  " << vd[q].position << " to vd. \n";
            insert_vertex_in_edge( q, q_edges[m] );
            assert( faces[left_face].type == INCIDENT);
            assert( faces[right_face].type == INCIDENT);
        }
    }
    std::cout << " vd vertex generation done \n";
}

// do this by face-traversal of the incident faces?
VertexVector VoronoiDiagram::traverse_v0_tree(VertexVector& v) {
    std::cout << " traverse_v0_tree() \n";
    VertexVector output;
    // start at v0
    VoronoiVertex current_vert = v[0];
    VoronoiVertex current_target;
    
    VoronoiOutEdgeItr it, it_end;
    boost::tie( it, it_end ) = boost::out_edges( current_vert, vd);
    VoronoiEdge current_edge = *it; // start at any edge
    
    // search for the first new vertex:
    bool found = false;
    VoronoiVertex first;
    std::cout << " stating search at vert = " << current_vert << " at edge = " << current_edge << "\n";
    while (!found) {
        current_target = boost::target( current_edge, vd);
        if ( vd[current_target].type == NEW ) {
            first = current_target;
            found = true;
        } else {
            current_edge = vd[current_edge].next;
        }
    }
    output.push_back(first);
    std::cout << " first = " << first << "\n";
    // now starting at first_new, traverse the tree and end when we encounter first again
    std::cout << " switching from " << current_edge << " to twin= " << vd[current_edge].twin << "\n";
    
    current_edge = vd[current_edge].twin; // we flip to the twin since we know target==NEW
    current_target = boost::target(current_edge, vd);
    std::cout << " current_target= " << current_target << "\n";
    
    while ( current_target != first ) {
        std::cout << "examining edge " << current_edge ;
        std::cout << " target has type " << vd[current_target].type << "\n";
        if ( vd[current_target].type == NEW ) {
            output.push_back(current_target);
            std::cout << " found NEW vertex "<< current_target << "\n";
            current_edge = vd[current_edge].twin;
        } else {
            current_edge = vd[current_edge].next;
        }
        current_target = boost::target(current_edge, vd);
    }
    
    return output;
}

void VoronoiDiagram::insert_vertex_in_edge(VoronoiVertex v, VoronoiEdge e) {
    // the vertex v is in the middle of edge e
    //                      face
    //                    e1   e2
    // previous-> source  -> v -> target
    //            tw_targ <- v <- tw_sour <- tw_previous
    //                    te2  te1
    //                     twin_face
    
    VoronoiEdge twin = vd[e].twin;
    VoronoiVertex source = boost::source( e , vd );
    VoronoiVertex target = boost::target( e , vd );
    VoronoiVertex twin_source = boost::source( twin , vd );
    VoronoiVertex twin_target = boost::target( twin , vd );
    
    FaceIdx face = vd[e].face;
    FaceIdx twin_face = vd[twin].face;
    VoronoiEdge previous = find_previous_edge(e);
    VoronoiEdge twin_previous = find_previous_edge(twin);
    
    VoronoiEdge e1 = add_edge( source, v );
    VoronoiEdge e2 = add_edge( v, target );
    
    // preserve the left/right face link
    vd[e1].face = face;
    vd[e1].next = e2;
    vd[e2].face = face;
    vd[e2].next = vd[e].next;
    vd[previous].next = e1;
    
    VoronoiEdge te1 = add_edge( twin_source, v );
    VoronoiEdge te2 = add_edge( v, twin_target );
    vd[te1].face = twin_face;
    vd[te1].next = te2;
    vd[te2].face = face;
    vd[te2].next = vd[twin].next;
    vd[twin_previous].next = te1;
    // TWINNING (note indices 'cross', see ASCII art above)
    vd[e1].twin = te2;
    vd[te2].twin = e1;
    vd[e2].twin = te1;
    vd[te1].twin = e2;
    
    // finally, remove the old edge
    boost::remove_edge( target, source, vd);
    boost::remove_edge( source, target, vd);
}

VoronoiEdge VoronoiDiagram::find_previous_edge(VoronoiEdge e) {
    VoronoiEdge previous = vd[e].next;
    while ( vd[previous].next != e ) {
        previous = vd[previous].next;
    }
    return previous;
}

FaceVector VoronoiDiagram::get_adjacent_faces( VoronoiVertex q ) {
    // given the vertex q, find the three adjacent faces
    std::set<FaceIdx> face_set;
    VoronoiOutEdgeItr itr, itr_end;
    boost::tie( itr, itr_end) = boost::out_edges(q, vd);
    for ( ; itr!=itr_end ; ++itr ) {
        face_set.insert( vd[*itr].face );
    }
    assert( face_set.size() == 3); // degree of q is three, so has three faces
    FaceVector fv;
    BOOST_FOREACH(FaceIdx m, face_set) {
        fv.push_back(m);
    }
    return fv;
}

void VoronoiDiagram::augment_vertex_set(VertexVector& q) {
    // RB2   voronoi-point q[0] = q( a, b, c ) is the seed
    // add faces Ca,Cb,Cc to the stack
    FaceVector adjacent_faces = get_adjacent_faces( q[0] );
    
    assert( adjacent_faces.size()==3 );
    std::stack<FaceIdx> S; // B1.3  we push all the adjacent faces onto the stack, and label them INCIDENT
    BOOST_FOREACH(FaceIdx f, adjacent_faces) {
        faces[f].type = INCIDENT;
        std::cout << " setting face= " << f << " INCIDENT \n";
        std::cout << " faces[f].type = " << faces[f].type  << "  \n";
        S.push(f);
    }
    while ( !S.empty() ) { 
        FaceIdx c_alfa = S.top();
        S.pop();
        std::cout << " augmenting from face = " << c_alfa << "\n";
        //VertexVector cycle_verts = get_generator_vertices(vd, dd, c_alfa); // get vertices of this cycle
        // B2.1  mark "out"  v in cycle_alfa if 
        //  (T6) v is adjacent to an IN vertex in V-Vin
        //  (T7) v is on an "incident" cycle other than cycle and is not adjacent to a vertex in Vin
        
        // B2.2 if subgraph (Vout,Eout) is disconnected, find minimal set V* of undecided vertices such that Vout U V* is connected
        // and set v=OUT for all V*
        
        // B2.3 if there is no "OUT" vertex in V(cycle) find vertkex v "undecided" with maximal H and put v=OUT
        
        // B2.4 while UNDECIDED vertices remain:
        //      B2.4.1 find an undecided vertex Qabc with maximal abs(H)
        //      B2.4.2 if H >= 0, put v=OUT.   if Vout disconnected, find V* of "undecided" vertices to make it connected
        //             and mark all V* OUT
        //      B2.4.3 if H < 0 set v=IN for a,b,c
        //         if c(j)=nonincident,  put c(j)=incident and add c(j) to the stack
        //         if Vin disconnected,
        //            (i) find subset C* to make it connected
        //            (ii) mark all of V* IN
        //            (iii) for nonincident cycles cj indcident on vertices in V*
        //                 put cj="incident" and add to stack.
        //
            
    }
}

VoronoiVertex VoronoiDiagram::find_seed_vertex(FaceIdx face_idx, const Point& p) {
    // evaluate H on all vertices along facet edges and return
    // vertex with the lowest H
    
    // find vertices that bound face_idx 
    VertexVector q_verts = get_face_vertices(face_idx);                 assert( q_verts.size() >= 3 );
    double minimumH = 1; // safe, because we expect the min H to be negative...
    VoronoiVertex minimalVertex;
    double h;
    BOOST_FOREACH( VoronoiVertex q, q_verts) {
        if ( vd[q].type != OUT ) {
            h = vd[q].detH( p ); 
            if (h<minimumH) { // find minimum H value among q_verts
                minimumH = h;
                minimalVertex = q;
            }
        }
    }
    assert( minimumH < 0 );
    return minimalVertex;
}

// traverse face and return all vertices found
VertexVector VoronoiDiagram::get_face_vertices(FaceIdx face_idx) {
    VertexVector verts;
    VoronoiEdge startedge = faces[face_idx].edge; // the edge where we start
    VoronoiVertex start_target = boost::target( startedge, vd); 
    verts.push_back(start_target);
    VoronoiEdge current = vd[startedge].next;
    do {
        VoronoiVertex current_target = boost::target( current, vd); 
        verts.push_back(current_target);
        current = vd[current].next;
    } while ( current != startedge );
    return verts;
}


unsigned int VoronoiDiagram::find_closest_face(const Point& p ) {
    FaceIdx closest_face;
    double closest_distance = 3*far_radius;
    double d;
    for (FaceIdx  m=0;m<faces.size();++m) {
        d = ( faces[m].generator - p).norm();
        if (d<closest_distance ) {
            closest_distance=d;
            closest_face=m;
        }
    }
    std::cout << " find_closest_Delaunay_vertex \n";
    std::cout << "   face id= " << closest_face << " at distance " << closest_distance << "\n";
    assert( closest_distance < 3*far_radius ) ;
    return closest_face;
}


boost::python::list VoronoiDiagram::getGenerators()  {
    boost::python::list plist;
    for ( FaceIdx m=0;m<faces.size();++m ) {
        plist.append( faces[m].generator );
    }
    return plist;
}

boost::python::list VoronoiDiagram::getVoronoiVertices() const {
    boost::python::list plist;
    VoronoiVertexItr it_begin, it_end, itr;
    boost::tie( it_begin, it_end ) = boost::vertices( vd );
    for ( itr=it_begin ; itr != it_end ; ++itr ) {
        plist.append( vd[*itr].position );
    }
    return plist;
}




boost::python::list VoronoiDiagram::getVoronoiEdges() const {
    return getEdges(vd);
}

boost::python::list VoronoiDiagram::getEdges(const VoronoiGraph& g) const {
    boost::python::list edge_list;
    VoronoiEdgeItr itr, it_end;
    boost::tie( itr, it_end ) = boost::edges( g );
    for (  ; itr != it_end ; ++itr ) { // loop through each edge
            boost::python::list point_list; // the endpoints of each edge
            VoronoiVertex v1 = boost::source( *itr, g  );
            VoronoiVertex v2 = boost::target( *itr, g  );
            point_list.append( g[v1].position );
            point_list.append( g[v2].position );
            edge_list.append(point_list);
    }
    return edge_list;
}




std::string VoronoiDiagram::str() const {
    std::ostringstream o;
    o << "VoronoiDiagram (nVerts="<< boost::num_vertices( vd ) << " , nEdges="<< boost::num_edges( vd ) <<"\n";
    
    return o.str();
}

} // end namespace
// end file weave.cpp
