/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

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
