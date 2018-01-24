//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Moose
#include "Moose.h"

// libMesh headers
#include "libmesh/fe_base.h"

// Forward declarations
class PenetrationInfo;

namespace Moose
{

void findContactPoint(PenetrationInfo & p_info,
                      FEBase * fe_elem,
                      FEBase * fe_side,
                      FEType & fe_side_type,
                      const Point & slave_point,
                      bool start_with_centroid,
                      const Real tangential_tolerance,
                      bool & contact_point_on_side);

void restrictPointToFace(Point & p, const Elem * side, std::vector<const Node *> & off_edge_nodes);
}
