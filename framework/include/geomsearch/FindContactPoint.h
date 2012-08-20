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
#include "PenetrationLocator.h"

// libMesh
#include "elem.h"
#include "vector_value.h"

namespace Moose
{

void
findContactPoint(PenetrationLocator::PenetrationInfo & p_info,
                 FEBase * _fe, FEType & _fe_type, const Point & slave_point,
                 bool start_with_centroid, const Real tangential_tolerance,
                 bool & contact_point_on_side);

void
restrictPointToFace(Point& p,
                    const Elem* side,
                    std::vector<Node*> &off_edge_nodes);

}

