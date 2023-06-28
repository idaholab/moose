//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVNernstTransport.h"

registerMooseObject("NavierStokesApp", INSFVNernstTransport);

InputParameters
INSFVNernstTransport::validParams()
{
  auto params = FVFluxKernel::validParams();
  params.addClassDescription("Computes residual for diffusion operator for finite volume method.");
  params.addRequiredParam<MooseFunctorName>("z", "the valence of the cation");
  params.addRequiredParam<MooseFunctorName>("temp", "temperature of the solution");
  params.addRequiredParam<MooseFunctorName>("phi", "The electrochemical potential");
  params.addRequiredParam<MooseFunctorName>("coeff", "diffusion coefficient");
  MooseEnum coeff_interp_method("average harmonic", "harmonic");
  params.addParam<MooseEnum>(
      "coeff_interp_method",
      coeff_interp_method,
      "Switch that can select face interpolation method for diffusion coefficients.");
  params.set<unsigned short>("ghost_layers") = 2;
  return params;
}

INSFVNernstTransport::INSFVNernstTransport(const InputParameters & params)
  : FVFluxKernel(params),
  _z(getFunctor<ADReal>("z")),
    _temp(getFunctor<ADReal>("temp")),
      _phi(getFunctor<ADReal>("phi")),
        _coeff(getFunctor<ADReal>("coeff")),
          _coeff_interp_method(
          Moose::FV::selectInterpolationMethod(getParam<MooseEnum>("coeff_interp_method")))
{
  if ((_var.faceInterpolationMethod() == Moose::FV::InterpMethod::SkewCorrectedAverage) &&
      (_tid == 0))
    adjustRMGhostLayers(std::max((unsigned short)(3), _pars.get<unsigned short>("ghost_layers")));
}

ADReal
INSFVNernstTransport::computeQpResidual()
{
  const auto state = determineState();
  ADReal z;
  ADReal temp;
  ADReal coeff;
  ADReal var_face;
  const auto face = makeCDFace(*_face_info);
  const auto grad_phi = _phi.gradient(face, state);
  // If we are on internal faces, we interpolate the diffusivity as usual
  if (_var.isInternalFace(*_face_info))
  {
    const ADReal z_elem = _z(elemArg(), state);
    const ADReal z_neighbor = _z(neighborArg(), state);
    const ADReal temp_elem = _temp(elemArg(), state);
    const ADReal temp_neighbor = _temp(neighborArg(), state);
    const ADReal coeff_elem = _coeff(elemArg(), state);
    const ADReal coeff_neighbor = _coeff(neighborArg(), state);
    const ADReal var_elem = _var(elemArg(), state);
    const ADReal var_neighbor = _var(neighborArg(), state);
    // If the diffusion coefficients are zero, then we can early return 0 (and avoid warnings if we
    // have a harmonic interpolation)
    if (!coeff_elem.value() && !coeff_neighbor.value())
      return 0;

    interpolate(_coeff_interp_method, z, z_elem, z_neighbor, *_face_info, true);
    interpolate(_coeff_interp_method, temp, temp_elem, temp_neighbor, *_face_info, true);
    interpolate(_coeff_interp_method, coeff, coeff_elem, coeff_neighbor, *_face_info, true);
    interpolate(_coeff_interp_method, var_face, var_elem, var_neighbor, *_face_info, true);
  }
  // Else we just use the boundary values (which depend on how the diffusion
  // coefficient is constructed)
  else
  {
    const auto face = singleSidedFaceArg();
    z = _z(face, state);
    temp = _temp(face, state);
    coeff = _coeff(face, state);
    var_face = _var(face, state);
  }
  return -_normal * ((96485.3 * z) / (8.3145 * temp)) * coeff * var_face * grad_phi;
}
