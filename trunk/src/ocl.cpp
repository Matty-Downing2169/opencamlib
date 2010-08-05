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
#include "ocl.h"
#include "revision.h"
#include <boost/python/docstring_options.hpp>

/// return the revision string
std::string revision() {
    return OCL_REV_STRING;
}

std::string ocl_docstring() {
    return "OpenCAMLib docstring";
}

/*
 *  Python wrapping
 */

using namespace ocl;

namespace bp = boost::python;

void export_cutters();
void export_geometry();
void export_cutsim();
void export_algo();

BOOST_PYTHON_MODULE(ocl) {
    bp::docstring_options doc_options(false, false);
    //doc_options.disable_all();
    //doc_options.disable_py_signatures();

    bp::def("revision", revision); // returns OCL revision string to python

    bp::def("__doc__", ocl_docstring);

    bp::def("eps", eps); // machine epsilon, see numeric.cpp

    export_geometry(); // see ocl_geometry.cpp

    export_cutters(); // see ocl_cutters.cpp

    export_cutsim(); // see ocl_cutsim.cpp
    
    export_algo(); // see ocl_algo.cpp

}



