//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once
// Moose
#include "Moose.h"
#include "MooseTypes.h"

// Forward declarations
class PenetrationInfo;
namespace libMesh
{
template <typename>
class FEGenericBase;
typedef FEGenericBase<Real> FEBase;
class FEType;
class Elem;
class Node;
class Point;
}

namespace Moose
{

void findContactPoint(PenetrationInfo & p_info,
                      libMesh::FEBase * fe_elem,
                      libMesh::FEBase * fe_side,
                      libMesh::FEType & fe_side_type,
                      const libMesh::Point & secondary_point,
                      bool start_with_centroid,
                      const Real tangential_tolerance,
                      bool & contact_point_on_side,
                      bool & search_succeeded);

void restrictPointToFace(libMesh::Point & p,
                         const libMesh::Elem * side,
                         std::vector<const libMesh::Node *> & off_edge_nodes);
}
