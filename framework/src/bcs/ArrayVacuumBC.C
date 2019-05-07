//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ArrayVacuumBC.h"

registerMooseObject("MooseApp", ArrayVacuumBC);

template <>
InputParameters
validParams<ArrayVacuumBC>()
{
  InputParameters params = validParams<ArrayIntegratedBC>();
  params.addParam<RealArrayValue>("alpha", "Ratio between directional gradient and solution");
  return params;
}

ArrayVacuumBC::ArrayVacuumBC(const InputParameters & parameters)
  : ArrayIntegratedBC(parameters),
    _alpha(isParamValid("alpha") ? getParam<RealArrayValue>("alpha") : RealArrayValue::Ones(_count))
{
  _alpha /= 2;
}

RealArrayValue
ArrayVacuumBC::computeQpResidual()
{
  return _alpha.cwiseProduct(_u[_qp]) * _test[_i][_qp];
}

RealArrayValue
ArrayVacuumBC::computeQpJacobian()
{
  return _test[_i][_qp] * _phi[_j][_qp] * _alpha;
}

RealArray
ArrayVacuumBC::computeQpOffDiagJacobian(MooseVariableFEBase & jvar)
{
  if (jvar.number() == _var.number())
  {
    RealArrayValue v = computeQpJacobian();
    RealArray t = RealArray::Zero(_var.count(), _var.count());
    t.diagonal() = v;
    return t;
  }
  else
    return RealArray(_var.count(), jvar.count());
}
