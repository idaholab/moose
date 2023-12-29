//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "StickyBC.h"
#include "MooseVariable.h"

registerMooseObject("TensorMechanicsApp", StickyBC);

InputParameters
StickyBC::validParams()
{
  InputParameters params = NodalBC::validParams();
  params.addParam<Real>(
      "min_value",
      std::numeric_limits<Real>::lowest(),
      "If the old variable value <= min_value, the variable is fixed at its old value");
  params.addParam<Real>(
      "max_value",
      std::numeric_limits<Real>::max(),
      "If the old variable value >= max_value, the variable is fixed at its old value");
  params.addClassDescription(
      "Imposes the boundary condition $u = u_{old}$ if $u_{old}$ exceeds the bounds provided");
  return params;
}

StickyBC::StickyBC(const InputParameters & parameters)
  : NodalBC(parameters),
    _u_old(_var.dofValuesOld()),
    _min_value(getParam<Real>("min_value")),
    _max_value(getParam<Real>("max_value"))
{
  if (_min_value > _max_value)
    mooseError("StickyBC: min_value must not be greater than max_value");
}

bool
StickyBC::shouldApply()
{
  const unsigned qp = 0; // this is a NodalBC: all qp = 0
  return (_u_old[qp] <= _min_value || _u_old[qp] >= _max_value);
}

Real
StickyBC::computeQpResidual()
{
  return _u[_qp] - _u_old[_qp];
}
