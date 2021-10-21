//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PCNSFVEnergyDiffusion.h"
#include "INSFVEnergyVariable.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", PCNSFVEnergyDiffusion);

InputParameters
PCNSFVEnergyDiffusion::validParams()
{
  auto params = FVFluxKernel::validParams();
  params.addClassDescription("Diffusion term in the porous media incompressible Navier-Stokes "
                             "fluid energy equations :  $-div(eps * k * grad(T))$");
  params.addRequiredCoupledVar(NS::porosity, "Porosity variable");
  params.addRequiredParam<MaterialPropertyName>(NS::k, "Thermal conductivity");
  params.addParam<bool>(
      "effective_diffusivity",
      false,
      "Whether the diffusivity should be multiplied by porosity, or whether the provided "
      "diffusivity is an effective diffusivity taking porosity effects into account");

  params.set<unsigned short>("ghost_layers") = 2;
  return params;
}

PCNSFVEnergyDiffusion::PCNSFVEnergyDiffusion(const InputParameters & params)
  : FVFluxKernel(params),
    _k_elem(getADMaterialProperty<Real>(NS::k)),
    _k_neighbor(getNeighborADMaterialProperty<Real>(NS::k)),
    _eps(coupledValue(NS::porosity)),
    _eps_neighbor(coupledNeighborValue(NS::porosity)),
    _porosity_factored_in(getParam<bool>("effective_diffusivity"))
{
#ifndef MOOSE_GLOBAL_AD_INDEXING
  mooseError("PINSFV is not supported by local AD indexing. In order to use PINSFV, please run "
             "the configure script in the root MOOSE directory with the configure option "
             "'--with-ad-indexing-type=global'");
#endif
  if (!dynamic_cast<INSFVEnergyVariable *>(&_var))
    mooseError("PCNSFVEnergyDiffusion may only be used with a fluid temperature variable, "
               "of variable type INSFVEnergyVariable.");
}

ADReal
PCNSFVEnergyDiffusion::computeQpResidual()
{
  // Interpolate thermal conductivity times porosity on the face
  ADReal k_eps_face;
  if (!_porosity_factored_in)
    interpolate(Moose::FV::InterpMethod::Average,
                k_eps_face,
                _k_elem[_qp] * _eps[_qp],
                _k_neighbor[_qp] * _eps_neighbor[_qp],
                *_face_info,
                true);
  else
    interpolate(Moose::FV::InterpMethod::Average,
                k_eps_face,
                _k_elem[_qp],
                _k_neighbor[_qp],
                *_face_info,
                true);

  // Compute the temperature gradient dotted with the surface normal
  auto dTdn = gradUDotNormal();

  return -k_eps_face * dTdn;
}
