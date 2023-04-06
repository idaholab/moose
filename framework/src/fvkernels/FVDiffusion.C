//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVDiffusion.h"

registerMooseObject("MooseApp", FVDiffusion);

InputParameters
FVDiffusion::validParams()
{
  InputParameters params = FVFluxKernel::validParams();
  params.addClassDescription("Computes residual for diffusion operator for finite volume method.");
  params.addRequiredParam<MooseFunctorName>("coeff", "diffusion coefficient");
  MooseEnum coeff_interp_method("average harmonic", "harmonic");
  params.addParam<MooseEnum>(
      "coeff_interp_method",
      coeff_interp_method,
      "Switch that can select face interpolation method for diffusion coefficients.");
  params.set<unsigned short>("ghost_layers") = 2;

  return params;
}

FVDiffusion::FVDiffusion(const InputParameters & params)
  : FVFluxKernel(params),
    _coeff(getFunctor<ADReal>("coeff")),
    _coeff_interp_method(
        Moose::FV::selectInterpolationMethod(getParam<MooseEnum>("coeff_interp_method")))
{
  if ((_var.faceInterpolationMethod() == Moose::FV::InterpMethod::SkewCorrectedAverage) &&
      (_tid == 0))
    adjustRMGhostLayers(std::max((unsigned short)(3), _pars.get<unsigned short>("ghost_layers")));
}

ADReal
FVDiffusion::computeQpResidual()
{
  using namespace Moose::FV;
  const auto state = determineState();

  auto dudn = gradUDotNormal(state);
  ADReal coeff;

  // If we are on internal faces, we interpolate the diffusivity as usual
  if (_var.isInternalFace(*_face_info))
    interpolate(_coeff_interp_method,
                coeff,
                _coeff(elemArg(), state),
                _coeff(neighborArg(), state),
                *_face_info,
                true);
  // Else we just use the boundary values (which depend on how the diffusion
  // coefficient is constructed)
  else
  {
    const auto face = singleSidedFaceArg();
    coeff = _coeff(face, state);
  }

  return -1 * coeff * dudn;
}
