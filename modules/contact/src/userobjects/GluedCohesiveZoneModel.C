//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GluedCohesiveZoneModel.h"
#include "MooseVariableFE.h"
#include "SystemBase.h"
#include "MortarUtils.h"
#include "MooseUtils.h"
#include "MathUtils.h"

#include "MortarContactUtils.h"
#include "FactorizedRankTwoTensor.h"

#include "ADReal.h"
#include "CohesiveZoneModelTools.h"
#include <Eigen/Core>

registerMooseObject("ContactApp", GluedCohesiveZoneModel);

InputParameters
GluedCohesiveZoneModel::validParams()
{
  InputParameters params = CohesiveZoneModelBase::validParams();

  params.addClassDescription("Computes the bilinear mixed mode cohesive zone model.");

  params.addRangeCheckedParam<Real>(
      "penalty_stiffness", "penalty_stiffness > 0.0", "Penalty stiffness for CZM.");
  return params;
}

GluedCohesiveZoneModel::GluedCohesiveZoneModel(const InputParameters & parameters)
  : WeightedGapUserObject(parameters),
    PenaltyWeightedGapUserObject(parameters),
    WeightedVelocitiesUserObject(parameters),
    CohesiveZoneModelBase(parameters),
    _penalty_stiffness_czm(getParam<Real>("penalty_stiffness"))
{
}

void
GluedCohesiveZoneModel::computeCZMTraction(const Node * const node)
{
  // First call does not have maps available
  const bool return_boolean = _dof_to_weighted_gap.find(node) == _dof_to_weighted_gap.end();
  if (return_boolean)
    return;

  // Split displacement jump into active and inactive parts
  const auto interface_displacement_jump =
      normalizeQuantity(_dof_to_interface_displacement_jump, node);

  // This traction vector is local at this point.
  _dof_to_czm_traction[node] = _penalty_stiffness_czm * interface_displacement_jump;
}

Real
GluedCohesiveZoneModel::getLocalDisplacementNormal(const Node * const node) const
{
  const auto it = _dof_to_interface_displacement_jump.find(_subproblem.mesh().nodePtr(node->id()));
  const auto it2 = _dof_to_weighted_gap.find(_subproblem.mesh().nodePtr(node->id()));

  if (it != _dof_to_interface_displacement_jump.end() && it2 != _dof_to_weighted_gap.end())
    return MetaPhysicL::raw_value(it->second(0) / it2->second.second);
  else
    return 0.0;
}

Real
GluedCohesiveZoneModel::getLocalDisplacementTangential(const Node * const node) const
{
  const auto it = _dof_to_interface_displacement_jump.find(_subproblem.mesh().nodePtr(node->id()));
  const auto it2 = _dof_to_weighted_gap.find(_subproblem.mesh().nodePtr(node->id()));

  if (it != _dof_to_interface_displacement_jump.end() && it2 != _dof_to_weighted_gap.end())
    return MetaPhysicL::raw_value(it->second(1) / it2->second.second);
  else
    return 0.0;
}
