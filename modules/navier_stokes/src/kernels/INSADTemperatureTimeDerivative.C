//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSADTemperatureTimeDerivative.h"

registerADMooseObject("NavierStokesApp", INSADTemperatureTimeDerivative);

defineADValidParams(
    INSADTemperatureTimeDerivative,
    ADTimeKernel,
    params.addClassDescription("This class computes the time derivative for the incompressible "
                               "Navier-Stokes momentum equation.");
    params.addParam<MaterialPropertyName>("rho_name", "rho", "density name");
    params.addParam<MaterialPropertyName>("cp_name", "cp", "specific heat name"););

template <ComputeStage compute_stage>
INSADTemperatureTimeDerivative<compute_stage>::INSADTemperatureTimeDerivative(
    const InputParameters & parameters)
  : ADTimeKernel<compute_stage>(parameters),
    _rho(adGetADMaterialProperty<Real>("rho_name")),
    _cp(adGetADMaterialProperty<Real>("cp_name"))
{
}

template <ComputeStage compute_stage>
ADResidual
INSADTemperatureTimeDerivative<compute_stage>::computeQpResidual()
{
  return _test[_i][_qp] * _rho[_qp] * _cp[_qp] * _u_dot[_qp];
}
