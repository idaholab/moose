//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DiscreteNucleationForce.h"
#include "DiscreteNucleationMap.h"

registerMooseObject("PhaseFieldApp", DiscreteNucleationForce);

InputParameters
DiscreteNucleationForce::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription(
      "Term for inserting grain nuclei or phases in non-conserved order parameter fields");
  params.addRequiredParam<UserObjectName>("map", "DiscreteNucleationMap user object");
  params.addParam<Real>("no_nucleus_value", 0.0, "Variable value indicating no nucleus is present");
  params.addParam<Real>(
      "nucleus_value", 1.0, "Variable value indicating the presence of a nucleus");
  return params;
}

DiscreteNucleationForce::DiscreteNucleationForce(const InputParameters & params)
  : Kernel(params),
    _map(getUserObject<DiscreteNucleationMap>("map")),
    _v0(getParam<Real>("no_nucleus_value")),
    _v1(getParam<Real>("nucleus_value"))
{
}

void
DiscreteNucleationForce::precalculateResidual()
{
  // check if a nucleation event list is available for the current element
  _nucleus = &_map.nuclei(_current_elem);
}

Real
DiscreteNucleationForce::computeQpResidual()
{
  return -((*_nucleus)[_qp] * (_v1 - _v0) + _v0) * _test[_i][_qp];
}
