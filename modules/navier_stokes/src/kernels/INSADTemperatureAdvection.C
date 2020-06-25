//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSADTemperatureAdvection.h"

registerMooseObject("NavierStokesApp", INSADTemperatureAdvection);

InputParameters
INSADTemperatureAdvection::validParams()
{
  InputParameters params = ADKernelValue::validParams();
  params.addClassDescription("This class computes the residual and Jacobian contributions for "
                             "temperature advection for a divergence free velocity field.");
  return params;
}

INSADTemperatureAdvection::INSADTemperatureAdvection(const InputParameters & parameters)
  : ADKernelValue(parameters),
    _temperature_convective_strong_residual(
        getADMaterialProperty<Real>("temperature_convective_strong_residual"))
{
}

ADReal
INSADTemperatureAdvection::precomputeQpResidual()
{
  return _temperature_convective_strong_residual[_qp];
}
