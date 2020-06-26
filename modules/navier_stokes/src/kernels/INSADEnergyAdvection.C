//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSADEnergyAdvection.h"

registerMooseObject("NavierStokesApp", INSADEnergyAdvection);

InputParameters
INSADEnergyAdvection::validParams()
{
  InputParameters params = ADKernelValue::validParams();
  params.addClassDescription("This class computes the residual and Jacobian contributions for "
                             "temperature advection for a divergence free velocity field.");
  return params;
}

INSADEnergyAdvection::INSADEnergyAdvection(const InputParameters & parameters)
  : ADKernelValue(parameters),
    _temperature_advective_strong_residual(
        getADMaterialProperty<Real>("temperature_advective_strong_residual"))
{
}

ADReal
INSADEnergyAdvection::precomputeQpResidual()
{
  return _temperature_advective_strong_residual[_qp];
}
