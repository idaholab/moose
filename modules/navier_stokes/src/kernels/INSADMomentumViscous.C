//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSADMomentumViscous.h"

registerADMooseObject("NavierStokesApp", INSADMomentumViscous);

defineADValidParams(
    INSADMomentumViscous,
    ADVectorKernelGrad,
    params.addClassDescription("Adds the viscous term to the INS momentum equation");
    params.addParam<MaterialPropertyName>("mu_name",
                                          "mu",
                                          "The name of the viscosity material property"););

template <ComputeStage compute_stage>
INSADMomentumViscous<compute_stage>::INSADMomentumViscous(const InputParameters & parameters)
  : ADVectorKernelGrad<compute_stage>(parameters), _mu(adGetADMaterialProperty<Real>("mu_name"))
{
}

template <ComputeStage compute_stage>
ADTensorResidual
INSADMomentumViscous<compute_stage>::precomputeQpResidual()
{
  return _mu[_qp] * _grad_u[_qp];
}
