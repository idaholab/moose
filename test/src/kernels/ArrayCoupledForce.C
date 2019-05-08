//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ArrayCoupledForce.h"

#include "MooseVariable.h"

registerMooseObject("MooseApp", ArrayCoupledForce);

template <>
InputParameters
validParams<ArrayCoupledForce>()
{
  InputParameters params = validParams<ArrayKernel>();

  params.addRequiredCoupledVar("v", "The coupled variable which provides the force");
  params.addRequiredParam<RealArrayValue>(
      "coef", "Coefficent ($\\sigma$) multiplier for the coupled force term.");

  return params;
}

ArrayCoupledForce::ArrayCoupledForce(const InputParameters & parameters)
  : ArrayKernel(parameters),
    _v_var(coupled("v")),
    _v(coupledValue("v")),
    _coef(getParam<RealArrayValue>("coef"))
{
  if (_var.number() == _v_var)
    mooseError("Coupled variable 'v' needs to be different from 'variable' with CoupledForce, "
               "consider using Reaction or somethig similar");
  if (getVar("v", 0)->count() > 1)
    mooseError("We are testing the coupling of a standard variable to an array variable");
}

RealArrayValue
ArrayCoupledForce::computeQpResidual()
{
  return -_coef * (_v[_qp] * _test[_i][_qp]);
}

RealArray
ArrayCoupledForce::computeQpOffDiagJacobian(MooseVariableFEBase & jvar)
{
  if (jvar.number() == _v_var)
    return _coef * (-_phi[_j][_qp] * _test[_i][_qp]);
  else
    return RealArray::Zero(_var.count(), jvar.count());
}
