//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PINSFVEnergyDiffusion.h"
#include "INSFVEnergyVariable.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", PINSFVEnergyDiffusion);

InputParameters
PINSFVEnergyDiffusion::validParams()
{
  auto params = FVFluxKernel::validParams();
  params += FVDiffusionInterpolationInterface::validParams();
  params.addClassDescription("Diffusion term in the porous media incompressible Navier-Stokes "
                             "fluid energy equations :  $-div(eps * k * grad(T))$");
  params.addRequiredParam<MooseFunctorName>(NS::porosity, "Porosity");
  params.addRequiredParam<MooseFunctorName>(NS::k, "Thermal conductivity");
  params.addParam<bool>(
      "effective_diffusivity",
      false,
      "Whether the conductivity should be multiplied by porosity, or whether the provided "
      "conductivity is an effective conductivity taking porosity effects into account");
  params.renameParam("effective_diffusivity", "effective_conductivity", "");
  MooseEnum coeff_interp_method("average harmonic", "harmonic");
  params.addParam<MooseEnum>(
      "kappa_interp_method",
      coeff_interp_method,
      "Switch that can select face interpolation method for the thermal conductivity.");

  // We add the relationship manager here, this will select the right number of
  // ghosting layers depending on the chosen interpolation method
  params.addRelationshipManager(
      "ElementSideNeighborLayers",
      Moose::RelationshipManagerType::GEOMETRIC | Moose::RelationshipManagerType::ALGEBRAIC |
          Moose::RelationshipManagerType::COUPLING,
      [](const InputParameters & obj_params, InputParameters & rm_params)
      { FVRelationshipManagerInterface::setRMParamsDiffusion(obj_params, rm_params, 3); });

  params.set<unsigned short>("ghost_layers") = 2;
  return params;
}

PINSFVEnergyDiffusion::PINSFVEnergyDiffusion(const InputParameters & params)
  : FVFluxKernel(params),
    SolutionInvalidInterface(this),
    FVDiffusionInterpolationInterface(params),
    _k(getFunctor<ADReal>(NS::k)),
    _eps(getFunctor<ADReal>(NS::porosity)),
    _porosity_factored_in(getParam<bool>("effective_conductivity")),
    _k_interp_method(
        Moose::FV::selectInterpolationMethod(getParam<MooseEnum>("kappa_interp_method")))
{
  if (!dynamic_cast<INSFVEnergyVariable *>(&_var))
    mooseError("PINSFVEnergyDiffusion may only be used with a fluid temperature variable, "
               "of variable type INSFVEnergyVariable.");
}

ADReal
PINSFVEnergyDiffusion::computeQpResidual()
{
  // Interpolate thermal conductivity times porosity on the face
  ADReal k_eps_face;
  const auto state = determineState();

  if (onBoundary(*_face_info))
  {
    const auto ssf = singleSidedFaceArg();
    k_eps_face = _porosity_factored_in ? _k(ssf, state) : _k(ssf, state) * _eps(ssf, state);
  }
  else
  {
    const auto face_elem = elemArg();
    const auto face_neighbor = neighborArg();

    auto value1 = _porosity_factored_in ? _k(face_elem, state)
                                        : _k(face_elem, state) * _eps(face_elem, state);
    auto value2 = _porosity_factored_in ? _k(face_neighbor, state)
                                        : _k(face_neighbor, state) * _eps(face_neighbor, state);

    // Adapt to users either passing 0 thermal conductivity, 0 porosity, or k correlations going
    // negative. The solution is invalid only for the latter case.
    auto k_interp_method = _k_interp_method;
    if (value1 <= 0 || value2 <= 0)
    {
      flagInvalidSolution(
          "Negative or null thermal conductivity value. If this is on purpose use arithmetic mean "
          "interpolation instead of the default harmonic interpolation.");
      if (_k_interp_method == Moose::FV::InterpMethod::HarmonicAverage)
        k_interp_method = Moose::FV::InterpMethod::Average;
      if (value1 < 0)
        value1 = 0;
      if (value2 < 0)
        value2 = 0;
    }

    Moose::FV::interpolate(k_interp_method, k_eps_face, value1, value2, *_face_info, true);
  }

  // Compute the temperature gradient dotted with the surface normal
  auto dTdn = gradUDotNormal(state, _correct_skewness);

  return -k_eps_face * dTdn;
}
