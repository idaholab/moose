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
                               "-\\nabla \\cdot \\frac{\\mathbf{K}}{\\mu} \\nabla p = 0");

    // Add a required parameter.  If this isn't provided in the input file MOOSE will error.
    params.addRequiredParam<Real>("permeability", "The permeability ($\\mathrm{K}$) of the fluid.");

    // Add a parameter with a default value; this value can be overridden in the input file.
    params.addParam<Real>(
        "viscosity",
        7.98e-4,
        "The viscosity ($\\mu$) of the fluid in Pa, the default is for water at 30 degrees C."););

template <ComputeStage compute_stage>
DarcyPressure<compute_stage>::DarcyPressure(const InputParameters & parameters)
  : ADDiffusion<compute_stage>(parameters),

    // Get the parameters from the input file
    _permeability(adGetParam<Real>("permeability")), //#define adGetParam this->template getParam
    _viscosity(adGetParam<Real>("viscosity"))
{
}

template <ComputeStage compute_stage>
ADVectorResidual
DarcyPressure<compute_stage>::precomputeQpResidual()
{
  return (_permeability / _viscosity) * ADDiffusion<compute_stage>::precomputeQpResidual();
}
