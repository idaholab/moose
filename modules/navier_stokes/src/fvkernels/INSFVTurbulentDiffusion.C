//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVTurbulentDiffusion.h"

registerMooseObject("MooseApp", INSFVTurbulentDiffusion);

InputParameters
INSFVTurbulentDiffusion::validParams()
{
  InputParameters params = FVDiffusion::validParams();
  params.addClassDescription(
      "Computes residual for the turbulent scaled diffusion operator for finite volume method.");
  params.addParam<MooseFunctorName>("scaling_coef", 1.0, "diffusion coefficient");
  params.set<unsigned short>("ghost_layers") = 2;

  return params;
}

INSFVTurbulentDiffusion::INSFVTurbulentDiffusion(const InputParameters & params)
  : FVDiffusion(params), _scaling_coef(getFunctor<ADReal>("scaling_coef"))
{
}

ADReal
INSFVTurbulentDiffusion::computeQpResidual()
{
  using namespace Moose::FV;

  auto dudn = gradUDotNormal();
  ADReal coeff;
  ADReal scaling_coef;

  // If we are on internal faces, we interpolate the diffusivity as usual
  if (_var.isInternalFace(*_face_info))
  {
    interpolate(
        _coeff_interp_method, coeff, _coeff(elemArg()), _coeff(neighborArg()), *_face_info, true);
    interpolate(_coeff_interp_method,
                scaling_coef,
                _scaling_coef(elemArg()),
                _scaling_coef(neighborArg()),
                *_face_info,
                true);
  }
  // Else we just use the boundary values (which depend on how the diffusion
  // coefficient is constructed)
  else
  {
    const auto face = singleSidedFaceArg();
    coeff = _coeff(face);
    scaling_coef = _scaling_coef(face);
  }

  return -1 * coeff / scaling_coef * dudn;
}
