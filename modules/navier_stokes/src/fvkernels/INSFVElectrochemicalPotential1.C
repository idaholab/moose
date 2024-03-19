//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVElectrochemicalPotential1.h"

registerMooseObject("NavierStokesTestApp", INSFVElectrochemicalPotential1);

InputParameters
INSFVElectrochemicalPotential1::validParams()
{
  auto params = FVFluxKernel::validParams();
  params.addClassDescription("Computes residual for diffusion operator for finite volume method.");
  params.addRequiredParam<MooseFunctorName>("epsilonr", "relative permittivity");
  MooseEnum coeff_interp_method("average harmonic", "harmonic");
  params.addParam<MooseEnum>(
      "coeff_interp_method",
      coeff_interp_method,
      "Switch that can select face interpolation method for diffusion coefficients.");
  params.set<unsigned short>("ghost_layers") = 2;
  return params;
}

INSFVElectrochemicalPotential1::INSFVElectrochemicalPotential1(const InputParameters & params)
  : FVFluxKernel(params),
    _epsilonr(getFunctor<ADReal>("epsilonr")),
      _coeff_interp_method(
          Moose::FV::selectInterpolationMethod(getParam<MooseEnum>("coeff_interp_method")))
{
  if ((_var.faceInterpolationMethod() == Moose::FV::InterpMethod::SkewCorrectedAverage) &&
      (_tid == 0))
    adjustRMGhostLayers(std::max((unsigned short)(3), _pars.get<unsigned short>("ghost_layers")));
}

ADReal
INSFVElectrochemicalPotential1::computeQpResidual()
{
  const auto face = makeCDFace(*_face_info);
  const auto state = determineState();
  ADReal epsilonr;
  const auto grad_phi = _var.gradient(face, state);
  // If we are on internal faces, we interpolate the diffusivity as usual
  if (_var.isInternalFace(*_face_info))
  {
    const ADReal epsilonr_elem = _epsilonr(elemArg(), state);
    const ADReal epsilonr_neighbor = _epsilonr(neighborArg(), state);
    // If the diffusion coefficients are zero, then we can early return 0 (and avoid warnings if we
    // have a harmonic interpolation)
    if (!epsilonr_elem.value() && !epsilonr_neighbor.value()) 
      return 0;

    interpolate(_coeff_interp_method, epsilonr, epsilonr_elem, epsilonr_neighbor, *_face_info, true);
  }
  // Else we just use the boundary values (which depend on how the diffusion
  // coefficient is constructed)
  else
  {
    epsilonr = _epsilonr(face, state);
  }
  return -_normal * (epsilonr * 8.8542e-12 * grad_phi);
}
