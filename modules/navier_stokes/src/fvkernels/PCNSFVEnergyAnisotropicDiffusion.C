//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PCNSFVEnergyAnisotropicDiffusion.h"
#include "INSFVEnergyVariable.h"

registerMooseObject("NavierStokesApp", PCNSFVEnergyAnisotropicDiffusion);
registerMooseObjectRenamed("MooseApp",
                           PINSFVEnergyEffectiveDiffusion,
                           "12/31/2021 00:01",
                           PCNSFVEnergyAnisotropicDiffusion);

InputParameters
PCNSFVEnergyAnisotropicDiffusion::validParams()
{
  auto params = FVFluxKernel::validParams();
  params.addClassDescription(
      "Anisotropic diffusion term in the porous media incompressible Navier-Stokes "
      "equations : -div(kappa grad(T))");
  params.addRequiredParam<MaterialPropertyName>(NS::kappa, "Vector of effective thermal conductivity");
  params.addRequiredCoupledVar(NS::porosity, "Porosity variable");
  params.addParam<bool>("effective_diffusivity", true, "Whether the diffusivity should be "
      "multiplied by porosity, or whether the provided diffusivity is an effective diffusivity "
      "taking porosity effects into account");

  params.set<unsigned short>("ghost_layers") = 2;
  return params;
}

PCNSFVEnergyAnisotropicDiffusion::PCNSFVEnergyAnisotropicDiffusion(const InputParameters & params)
  : FVFluxKernel(params),
    _k_elem(getADMaterialProperty<RealVectorValue>(NS::kappa)),
    _k_neighbor(getNeighborADMaterialProperty<RealVectorValue>(NS::kappa)),
    _eps(coupledValue(NS::porosity)),
    _eps_neighbor(coupledNeighborValue(NS::porosity)),
    _porosity_factored_in(getParam<bool>("effective_diffusivity"))
{
#ifndef MOOSE_GLOBAL_AD_INDEXING
  mooseError("PINSFV is not supported by local AD indexing. In order to use PINSFV, please run the "
             "configure script in the root MOOSE directory with the configure option "
             "'--with-ad-indexing-type=global'");
#endif
  if (!dynamic_cast<INSFVEnergyVariable *>(&_var))
    mooseError("PCNSFVEnergyAnisotropicDiffusion may only be used with a fluid temperature variable, "
               "of variable type INSFVEnergyVariable.");
}

ADReal
PCNSFVEnergyAnisotropicDiffusion::computeQpResidual()
{
  // Interpolate thermal conductivity times porosity on the face
  ADRealVectorValue k_eps_face;
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

  // Compute the temperature gradient times the conductivity tensor
  ADRealVectorValue kappa_grad_T;
  const auto grad_T = _var.adGradSln(*_face_info);
  for (std::size_t i = 0; i < LIBMESH_DIM; i++)
    kappa_grad_T(i) = k_eps_face(i) * grad_T(i);

  return -kappa_grad_T * _normal;
}
