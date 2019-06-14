//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DarcyPressure.h"

registerADMooseObject("DarcyThermoMechApp", DarcyPressure);

defineADValidParams(
    DarcyPressure,
    ADDiffusion,
    params.addClassDescription("Compute the diffusion term for Darcy pressure ($p$) equation: "
                               "-\\nabla \\cdot \\frac{\\mathbf{K}}{\\mu} \\nabla p = 0"););

template <ComputeStage compute_stage>
DarcyPressure<compute_stage>::DarcyPressure(const InputParameters & parameters)
  : ADDiffusion<compute_stage>(parameters),
    _permeability(adGetMaterialProperty<Real>("permeability")),
    _viscosity(adGetADMaterialProperty<Real>("viscosity"))
{
}

template <ComputeStage compute_stage>
ADVectorResidual
DarcyPressure<compute_stage>::precomputeQpResidual()
{
  return (_permeability[_qp] / _viscosity[_qp]) *
         ADDiffusion<compute_stage>::precomputeQpResidual();
}
