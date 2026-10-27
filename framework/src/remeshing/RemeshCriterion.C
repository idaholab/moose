//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RemeshCriterion.h"

#include "DisplacedProblem.h"
#include "FEProblemBase.h"
#include "MooseMesh.h"

InputParameters
RemeshCriterion::validParams()
{
  InputParameters params = MooseObject::validParams();
  params.registerBase("RemeshCriterion");
  return params;
}

RemeshCriterion::RemeshCriterion(const InputParameters & parameters)
  : MooseObject(parameters),
    SetupInterface(this),
    _fe_problem(*getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _mesh(_fe_problem.mesh()),
    _remeshing(_fe_problem.getRemeshing())
{
}

bool
RemeshCriterion::minimumBelowThreshold(Real local_value, const Real threshold) const
{
  _communicator.min(local_value);
  return local_value < threshold;
}

bool
RemeshCriterion::maximumAboveThreshold(Real local_value, const Real threshold) const
{
  _communicator.max(local_value);
  return local_value > threshold;
}

MooseMesh &
RemeshCriterion::evaluationMesh() const
{
  if (_remeshing.displacements().empty())
    return _mesh;

  const auto displaced_problem = _fe_problem.getDisplacedProblem();
  if (!displaced_problem)
    mooseError("The [Remeshing] block was given displacement variables, but the problem has no "
               "displaced mesh to evaluate this criterion on.");

  return displaced_problem->mesh();
}

void
RemeshCriterion::requireMeshMovement() const
{
  if (!_remeshing.meshMovementEnabled())
    mooseError("This criterion measures the pseudo-displacement accumulated since the last remesh, "
               "which is identically zero unless the [Remeshing] block sets mesh_movement = true. "
               "Set mesh_movement = true, or remove this criterion.");
}

const Remeshing::PointMap &
RemeshCriterion::pseudoDisplacement() const
{
  requireMeshMovement();

  return _remeshing.pseudoDisplacement();
}
