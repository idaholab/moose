//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVAnisotropicDiffusion.h"

registerMooseObject("MooseApp", FVAnisotropicDiffusion);

InputParameters
FVAnisotropicDiffusion::validParams()
{
  InputParameters params = FVFluxKernel::validParams();
  params.addClassDescription(
      "Computes residual for anisotropic diffusion operator for finite volume method.");
  params.addRequiredParam<MooseFunctorName>("coeff",
                                            "The diagonal coefficients of a diffusion tensor.");
  MooseEnum coeff_interp_method("average harmonic", "harmonic");
  params.addParam<MooseEnum>(
      "coeff_interp_method",
      coeff_interp_method,
      "Switch that can select face interpolation method for diffusion coefficients.");
  MooseEnum face_interp_method("average skewness-corrected", "average");
  params.addParam<MooseEnum>(
      "variable_interp_method",
      face_interp_method,
      "Switch that can select between face interpolation methods for the variable.");
  params.set<unsigned short>("ghost_layers") = 2;

  // We add the relationship manager here, this will select the right number of
  // ghosting layers depending on the chosen interpolation method
  params.addRelationshipManager(
      "ElementSideNeighborLayers",
      Moose::RelationshipManagerType::GEOMETRIC | Moose::RelationshipManagerType::ALGEBRAIC |
          Moose::RelationshipManagerType::COUPLING,
      [](const InputParameters & obj_params, InputParameters & rm_params)
      { FVRelationshipManagerInterface::setRMParamsDiffusion(obj_params, rm_params, 3); });

  return params;
}

FVAnisotropicDiffusion::FVAnisotropicDiffusion(const InputParameters & params)
  : FVFluxKernel(params),
    _coeff(getFunctor<ADRealVectorValue>("coeff")),
    _coeff_interp_method(
        Moose::FV::selectInterpolationMethod(getParam<MooseEnum>("coeff_interp_method"))),
    _var_interp_method(
        Moose::FV::selectInterpolationMethod(getParam<MooseEnum>("variable_interp_method"))),
    _correct_skewness(_var_interp_method == Moose::FV::InterpMethod::SkewCorrectedAverage)
{
}

ADReal
FVAnisotropicDiffusion::computeQpResidual()
{
  const auto state = determineState();
  const auto & grad_T = _var.adGradSln(*_face_info, state, _correct_skewness);

  ADRealVectorValue coeff;
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

  ADReal r = 0;
  for (const auto i : make_range(Moose::dim))
    r += _normal(i) * coeff(i) * grad_T(i);
  return -r;
}
