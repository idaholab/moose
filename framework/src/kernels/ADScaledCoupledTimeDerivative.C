//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADScaledCoupledTimeDerivative.h"

registerMooseObject("MooseApp", ADScaledCoupledTimeDerivative);

InputParameters
ADScaledCoupledTimeDerivative::validParams()
{
  auto params = ADCoupledTimeDerivative::validParams();
  params.addClassDescription(
      "Extension of the ADCoupledTimeDerivative kernel that calculates the time derivative "
      "of a coupled variable scaled by a material property");

  params.addRequiredParam<MaterialPropertyName>("mat_prop", "Name of the material property factor");
  return params;
}

ADScaledCoupledTimeDerivative::ADScaledCoupledTimeDerivative(const InputParameters & parameters)
  : ADCoupledTimeDerivative(parameters), _mat_prop(getADMaterialProperty<Real>("mat_prop"))

{
}

ADReal
ADScaledCoupledTimeDerivative::precomputeQpResidual()
{
  return _mat_prop[_qp] * ADCoupledTimeDerivative::precomputeQpResidual();
}
