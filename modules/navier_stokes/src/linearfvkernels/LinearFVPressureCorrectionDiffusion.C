//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVPressureCorrectionDiffusion.h"

registerMooseObject("NavierStokesApp", LinearFVPressureCorrectionDiffusion);

InputParameters
LinearFVPressureCorrectionDiffusion::validParams()
{
  InputParameters params = LinearFVAnisotropicDiffusion::validParams();
  params.addClassDescription("Adds the pressure correction diffusion term for the linear finite "
                             "volume SIMPLE algorithm.");
  params.suppressParameter<InterpolationMethodName>("coeff_interp_method");
  return params;
}

LinearFVPressureCorrectionDiffusion::LinearFVPressureCorrectionDiffusion(
    const InputParameters & params)
  : LinearFVAnisotropicDiffusion(params)
{
}
