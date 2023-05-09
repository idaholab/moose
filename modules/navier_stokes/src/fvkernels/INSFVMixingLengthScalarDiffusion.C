//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVMixingLengthScalarDiffusion.h"
#include "MathFVUtils.h"

registerMooseObject("NavierStokesApp", INSFVMixingLengthScalarDiffusion);

InputParameters
INSFVMixingLengthScalarDiffusion::validParams()
{
  InputParameters params = FVFluxKernel::validParams();
  params.addClassDescription("Computes the turbulent diffusive flux that appears in "
                             "Reynolds-averaged fluid conservation equations.");
  params.addRequiredParam<MooseFunctorName>("u", "The velocity in the x direction.");
  params.addParam<MooseFunctorName>("v", "The velocity in the y direction.");
  params.addParam<MooseFunctorName>("w", "The velocity in the z direction.");
  params.addRequiredParam<MooseFunctorName>("mixing_length", "The turbulent mixing length.");
  params.addRequiredParam<Real>(
      "schmidt_number",
      "The turbulent Schmidt number (or turbulent Prandtl number if the passive scalar is energy) "
      "that relates the turbulent scalar diffusivity to the turbulent momentum diffusivity.");
  params.set<unsigned short>("ghost_layers") = 2;
  return params;
}

INSFVMixingLengthScalarDiffusion::INSFVMixingLengthScalarDiffusion(const InputParameters & params)
  : FVFluxKernel(params),
    _dim(_subproblem.mesh().dimension()),
    _u(getFunctor<ADReal>("u")),
    _v(isParamValid("v") ? &getFunctor<ADReal>("v") : nullptr),
    _w(isParamValid("w") ? &getFunctor<ADReal>("w") : nullptr),
    _mixing_len(getFunctor<ADReal>("mixing_length")),
    _schmidt_number(getParam<Real>("schmidt_number"))
{
  if (_dim >= 2 && !_v)
    mooseError(
        "In two or more dimensions, the v velocity must be supplied using the 'v' parameter");
  if (_dim >= 3 && !_w)
    mooseError("In threedimensions, the w velocity must be supplied using the 'w' parameter");
}

ADReal
INSFVMixingLengthScalarDiffusion::computeQpResidual()
{
  constexpr Real offset = 1e-15; // prevents explosion of sqrt(x) derivative to infinity

  auto face = makeCDFace(*_face_info);
  const auto state = determineState();

  const auto grad_u = _u.gradient(face, state);
  ADReal symmetric_strain_tensor_norm = 2.0 * Utility::pow<2>(grad_u(0));
  if (_dim >= 2)
  {
    const auto grad_v = _v->gradient(face, state);
    symmetric_strain_tensor_norm +=
        2.0 * Utility::pow<2>(grad_v(1)) + Utility::pow<2>(grad_v(0) + grad_u(1));
    if (_dim >= 3)
    {
      const auto grad_w = _w->gradient(face, state);
      symmetric_strain_tensor_norm += 2.0 * Utility::pow<2>(grad_w(2)) +
                                      Utility::pow<2>(grad_u(2) + grad_w(0)) +
                                      Utility::pow<2>(grad_v(2) + grad_w(1));
    }
  }

  symmetric_strain_tensor_norm = std::sqrt(symmetric_strain_tensor_norm + offset);

  // Interpolate the mixing length to the face
  ADReal mixing_len = _mixing_len(face, state);

  // Compute the eddy diffusivity for momentum
  ADReal eddy_diff = symmetric_strain_tensor_norm * mixing_len * mixing_len;

  // Use the turbulent Schmidt/Prandtl number to get the eddy diffusivity for
  // the scalar variable
  eddy_diff /= _schmidt_number;

  // Compute the diffusive flux of the scalar variable
  auto dudn = gradUDotNormal(state);
  return -1 * eddy_diff * dudn;
}
