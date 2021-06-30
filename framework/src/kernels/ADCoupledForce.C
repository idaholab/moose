//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADCoupledForce.h"

registerMooseObject("MooseApp", ADCoupledForce);

InputParameters
ADCoupledForce::validParams()
{
  InputParameters params = ADKernel::validParams();

  params.addClassDescription("Implements a source term proportional to the value of a coupled "
                             "variable. Weak form: $(\\psi_i, -\\sigma v)$.");
  params.addRequiredCoupledVar("v", "The coupled variable which provides the force");
  params.addParam<Real>(
      "coef", 1.0, "Coefficent ($\\sigma$) multiplier for the coupled force term.");

  return params;
}

ADCoupledForce::ADCoupledForce(const InputParameters & parameters)
  : ADKernel(parameters),
    _v_var(coupled("v")),
    _v(adCoupledValue("v")),
    _coef(getParam<Real>("coef"))
{
  if (_var.number() == _v_var)
    mooseError("Coupled variable 'v' needs to be different from 'variable' with ADCoupledForce, "
               "consider using Reaction or somethig similar");
}

ADReal
ADCoupledForce::computeQpResidual()
{
  return -_coef * _v[_qp] * _test[_i][_qp];
}
