//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SimpleSplitCHWRes.h"

registerMooseObject("PhaseFieldApp", SimpleSplitCHWRes);

InputParameters
SimpleSplitCHWRes::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription("Gradient energy for split Cahn-Hilliard equation with constant "
                             "Mobility for a coupled order parameter variable.");
  params.addParam<MaterialPropertyName>(
      "mob_name", "M", "The mobility used with the kernel, should be a constant value");
  return params;
}

SimpleSplitCHWRes::SimpleSplitCHWRes(const InputParameters & parameters)
  : Kernel(parameters), _M(getMaterialProperty<Real>("mob_name"))
{
}

Real
SimpleSplitCHWRes::computeQpResidual()
{
  return _M[_qp] * _grad_u[_qp] * _grad_test[_i][_qp];
}

Real
SimpleSplitCHWRes::computeQpJacobian()
{
  return _M[_qp] * _grad_phi[_j][_qp] * _grad_test[_i][_qp];
}
