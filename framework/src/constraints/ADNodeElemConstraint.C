//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADNodeElemConstraint.h"

// MOOSE includes
#include "Assembly.h"
#include "MooseEnum.h"
#include "MooseMesh.h"
#include "MooseTypes.h"

#include "libmesh/string_to_enum.h"

InputParameters
ADNodeElemConstraint::validParams()
{
  InputParameters params = NodeElemConstraintBase::validParams();
  return params;
}

ADNodeElemConstraint::ADNodeElemConstraint(const InputParameters & parameters)
  : NodeElemConstraintBase(parameters),
    _u_primary(_primary_var.adSlnNeighbor()),
    _u_secondary(_var.adDofValues())
{
}

void
ADNodeElemConstraint::computeResidual()
{
  _qp = 0;

  _residuals.resize(_test_primary.size(), 0);
  for (auto & r : _residuals)
    r = 0;

  for (_i = 0; _i < _test_primary.size(); _i++)
    _residuals[_i] += raw_value(computeQpResidual(Moose::Primary));

  addResiduals(
      _assembly, _residuals, _primary_var.dofIndicesNeighbor(), _primary_var.scalingFactor());

  _residuals.resize(_test_secondary.size(), 0);

  for (auto & r : _residuals)
    r = 0;

  for (_i = 0; _i < _test_secondary.size(); _i++)
    _residuals[_i] += raw_value(computeQpResidual(Moose::Secondary));

  addResiduals(_assembly, _residuals, _var.dofIndices(), _var.scalingFactor());
}

void
ADNodeElemConstraint::computeJacobian()
{
  _qp = 0;

  std::vector<ADReal> primary_residual(_test_primary.size(), 0);

  for (_i = 0; _i < _test_primary.size(); _i++)
    primary_residual[_i] += computeQpResidual(Moose::Primary);

  addJacobian(_assembly, primary_residual, _primary_var.dofIndicesNeighbor(), _var.scalingFactor());

  std::vector<ADReal> secondary_residual(_test_secondary.size(), 0);

  for (_i = 0; _i < _test_secondary.size(); _i++)
    secondary_residual[_i] += computeQpResidual(Moose::Secondary);

  addJacobian(_assembly, secondary_residual, _var.dofIndices(), _var.scalingFactor());
}
