//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADMatCoupledForce.h"

registerMooseObject("MooseApp", ADMatCoupledForce);

InputParameters
ADMatCoupledForce::validParams()
{
  InputParameters params = ADCoupledForce::validParams();
  params.addClassDescription(
      "Kernel representing the contribution of the PDE term $mv$, where $m$ is a material property "
      "coefficient, $v$ is a coupled scalar field variable, and Jacobian derivatives are "
      "calculated "
      "using automatic differentiation.");
  params.addParam<MaterialPropertyName>(
      "mat_prop_coef",
      1.0,
      "User-supplied material property multiplier for the coupled force term.");
  return params;
}

ADMatCoupledForce::ADMatCoupledForce(const InputParameters & parameters)
  : ADCoupledForce(parameters), _mat_prop(getADMaterialProperty<Real>("mat_prop_coef"))
{
}

ADReal
ADMatCoupledForce::computeQpResidual()
{
  return _mat_prop[_qp] * ADCoupledForce::computeQpResidual();
}
