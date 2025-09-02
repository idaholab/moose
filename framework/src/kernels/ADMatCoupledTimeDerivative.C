//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADMatCoupledTimeDerivative.h"

registerMooseObject("MooseApp", ADMatCoupledTimeDerivative);

InputParameters
ADMatCoupledTimeDerivative::validParams()
{
  auto params = ADCoupledTimeDerivative::validParams();
  params.addClassDescription("Extension of the ADCoupledTimeDerivative kernel that consumes an "
                             "arbitrary material property");

  params.addRequiredParam<MaterialPropertyName>("mat_prop", "Name of the material property");
  return params;
}

ADMatCoupledTimeDerivative::ADMatCoupledTimeDerivative(const InputParameters & parameters)
  : ADCoupledTimeDerivative(parameters), _mat_prop(getADMaterialProperty<Real>("mat_prop"))

{
}

ADReal
ADMatCoupledTimeDerivative::precomputeQpResidual()
{
  return _mat_prop[_qp] * ADCoupledTimeDerivative::precomputeQpResidual();
}
