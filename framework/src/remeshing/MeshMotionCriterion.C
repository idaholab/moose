//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MeshMotionCriterion.h"

#include "FEProblemBase.h"
#include "MooseMesh.h"

#include "libmesh/elem.h"
#include "libmesh/utility.h"

#include <limits>

registerMooseObject("MooseApp", MeshMotionCriterion);

InputParameters
MeshMotionCriterion::validParams()
{
  InputParameters params = RemeshCriterion::validParams();

  params.addClassDescription("Remeshes when the pseudo-displacement accumulated since the last "
                             "remesh becomes large compared to the local element size.");

  params.addRequiredParam<Real>("threshold",
                                "The criterion fires when the largest nodal pseudo-displacement of "
                                "an active element, divided by the diameter of that element, is "
                                "above this.");

  return params;
}

MeshMotionCriterion::MeshMotionCriterion(const InputParameters & parameters)
  : RemeshCriterion(parameters), _threshold(getParam<Real>("threshold"))
{
}

void
MeshMotionCriterion::initialSetup()
{
  requireMeshMovement();
}

bool
MeshMotionCriterion::shouldRemesh()
{
  const auto & pseudo_displacement = pseudoDisplacement();

  Real local_maximum = std::numeric_limits<Real>::lowest();
  for (const auto & elem : evaluationMesh().getMesh().active_local_element_ptr_range())
  {
    Real motion = 0;
    for (const auto n : elem->node_index_range())
      motion = std::max(motion, libmesh_map_find(pseudo_displacement, elem->node_id(n)).norm());

    // Elem::hmax() is the largest vertex separation, which is the diameter of a straight sided
    // element
    local_maximum = std::max(local_maximum, motion / elem->hmax());
  }

  return maximumAboveThreshold(local_maximum, _threshold);
}
