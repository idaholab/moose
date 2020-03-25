//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADCoupledTimeDerivative.h"

registerMooseObject("MooseApp", ADCoupledTimeDerivative);

InputParameters
ADCoupledTimeDerivative::validParams()
{
  auto params = ADKernelValue::validParams();
  params.addClassDescription("Time derivative Kernel that acts on a coupled variable. Weak form: "
                             "$(\\psi_i, \\frac{\\partial v_h}{\\partial t})$.");
  params.addRequiredCoupledVar("v", "Coupled variable");
  return params;
}

ADCoupledTimeDerivative::ADCoupledTimeDerivative(const InputParameters & parameters)
  : ADKernelValue(parameters), _v_dot(adCoupledDot("v"))
{
}

ADReal
ADCoupledTimeDerivative::precomputeQpResidual()
{
  return _v_dot[_qp];
}
