//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
  MooseEnum face_interp_method("average skewness-corrected", "average");
  params.addParam<MooseEnum>("variable_interp_method",
                              face_interp_method,
                              "Switch that can select between face interpolation methods for the variable.");

  // We need at least 2 layers here with the least accurate interpolation
  params.set<unsigned short>("ghost_layers") = 2;

  // We add the relationship manager there, this will select the right number of
  // ghosting layers depending on the chosen interpolation method
  params.addRelationshipManager(
    "ElementSideNeighborLayers",
    Moose::RelationshipManagerType::GEOMETRIC | Moose::RelationshipManagerType::ALGEBRAIC |
        Moose::RelationshipManagerType::COUPLING,
    [](const InputParameters & obj_params, InputParameters & rm_params)
    {
      FVRelationshipManagerInterface::setRMParamsDiffusion(obj_params, rm_params, 3, true);
    });

  return params;
}

FVDiffusion::FVDiffusion(const InputParameters & params)
  : FVFluxKernel(params),
    _coeff(getFunctor<ADReal>("coeff")),
    _coeff_interp_method(
        Moose::FV::selectInterpolationMethod(getParam<MooseEnum>("coeff_interp_method"))),
    _var_interp_method(Moose::FV::selectInterpolationMethod(getParam<MooseEnum>("variable_interp_method"))),
    _correct_skewness(_var_interp_method == Moose::FV::InterpMethod::SkewCorrectedAverage)
{
}

ADReal
FVDiffusion::computeQpResidual()
{
  using namespace Moose::FV;
  const auto state = determineState();

  auto dudn = gradUDotNormal(state, _correct_skewness);
  ADReal coeff;

  // If we are on internal faces, we interpolate the diffusivity as usual
  if (_var.isInternalFace(*_face_info))
  {
    const ADReal coeff_elem = _coeff(elemArg(), state);
    const ADReal coeff_neighbor = _coeff(neighborArg(), state);
    // If the diffusion coefficients are zero, then we can early return 0 (and avoid warnings if we
    // have a harmonic interpolation)
    if (!coeff_elem.value() && !coeff_neighbor.value())
      return 0;

    interpolate(_coeff_interp_method, coeff, coeff_elem, coeff_neighbor, *_face_info, true);
  }
  // Else we just use the boundary values (which depend on how the diffusion
  // coefficient is constructed)
  else
  {
    const auto face = singleSidedFaceArg();
    coeff = _coeff(face, state);
  }

  return -1 * coeff * dudn;
}
