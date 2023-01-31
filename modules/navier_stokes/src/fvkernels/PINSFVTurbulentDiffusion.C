//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PINSFVTurbulentDiffusion.h"
#include "INSFVEnergyVariable.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", PINSFVTurbulentDiffusion);

InputParameters
PINSFVTurbulentDiffusion::validParams()
{
  auto params = FVFluxKernel::validParams();
  params.addClassDescription("Diffusion term in the porous media "
                             "partially incompressible Navier-Stokes turbulence"
                             "fluid energy equations :  $-div(eps * k / turb_coef * grad(T))$");
  params.addRequiredParam<MooseFunctorName>("mu_t", "Thermal conductivity");
  params.addRequiredParam<MooseFunctorName>(NS::porosity, "Porosity");
  params.addRequiredParam<MooseFunctorName>("turb_coef",
                                            "Normalization Coefficient Turbulent Diffusion");
  params.addParam<bool>(
      "effective_diffusivity",
      false,
      "Whether the diffusivity should be multiplied by porosity, or whether the provided "
      "diffusivity is an effective diffusivity taking porosity effects into account");

  params.set<unsigned short>("ghost_layers") = 2;
  return params;
}

PINSFVTurbulentDiffusion::PINSFVTurbulentDiffusion(const InputParameters & params)
  : FVFluxKernel(params),
    _mu_t(getFunctor<ADReal>("mu_t")),
    _eps(getFunctor<ADReal>(NS::porosity)),
    _turb_coef(getFunctor<ADReal>("turb_coef")),
    _porosity_factored_in(getParam<bool>("effective_diffusivity"))
{
#ifndef MOOSE_GLOBAL_AD_INDEXING
  mooseError("PINSFV is not supported by local AD indexing. In order to use PINSFV, please run "
             "the configure script in the root MOOSE directory with the configure option "
             "'--with-ad-indexing-type=global'");
#endif
  if (!dynamic_cast<INSFVEnergyVariable *>(&_var))
    mooseError("PINSFVTurbulentDiffusion may only be used with a fluid temperature variable, "
               "of variable type INSFVEnergyVariable.");
}

ADReal
PINSFVTurbulentDiffusion::computeQpResidual()
{
  // Interpolate thermal conductivity times porosity on the face
  ADReal diffusion_face;

  if (onBoundary(*_face_info))
  {
    const auto ssf = singleSidedFaceArg();
    diffusion_face = _porosity_factored_in ? _mu_t(ssf) * _eps(ssf) / _turb_coef(ssf)
                                           : _mu_t(ssf) / _turb_coef(ssf);
  }
  else
  {
    const auto face_elem = elemFromFace();
    const auto face_neighbor = neighborFromFace();

    if (!_porosity_factored_in)
      Moose::FV::interpolate(Moose::FV::InterpMethod::Average,
                             diffusion_face,
                             _mu_t(face_elem) * _eps(face_elem) / _turb_coef(face_elem),
                             _mu_t(face_neighbor) * _eps(face_neighbor) / _turb_coef(face_elem),
                             *_face_info,
                             true);
    else
      Moose::FV::interpolate(Moose::FV::InterpMethod::Average,
                             diffusion_face,
                             _mu_t(face_elem) / _turb_coef(face_elem),
                             _mu_t(face_neighbor) / _turb_coef(face_elem),
                             *_face_info,
                             true);
  }

  // Compute the gradient dotted with the surface normal
  auto dVardn = gradUDotNormal();

  return -diffusion_face * dVardn;
}
