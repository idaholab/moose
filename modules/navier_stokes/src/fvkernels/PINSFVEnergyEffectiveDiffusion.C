//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PINSFVEnergyEffectiveDiffusion.h"
#include "INSFVEnergyVariable.h"

registerMooseObject("NavierStokesApp", PINSFVEnergyEffectiveDiffusion);

InputParameters
PINSFVEnergyEffectiveDiffusion::validParams()
{
  auto params = FVFluxKernel::validParams();
  params.addClassDescription(
      "Effective diffusion term in the porous media incompressible Navier-Stokes "
      "equations : -div(kappa grad(T))");
  params.addParam<MaterialPropertyName>("kappa", "Vector of effective thermal conductivity");
  params.addRequiredCoupledVar("porosity", "Porosity variable");

  params.set<unsigned short>("ghost_layers") = 2;
  return params;
}

PINSFVEnergyEffectiveDiffusion::PINSFVEnergyEffectiveDiffusion(const InputParameters & params)
  : FVFluxKernel(params),
    _kappa_elem(getADMaterialProperty<RealVectorValue>("kappa")),
    _kappa_neighbor(getNeighborADMaterialProperty<RealVectorValue>("kappa")),
    _eps(coupledValue("porosity")),
    _eps_neighbor(coupledNeighborValue("porosity"))
{
#ifndef MOOSE_GLOBAL_AD_INDEXING
  mooseError("PINSFV is not supported by local AD indexing. In order to use PINSFV, please run the "
             "configure script in the root MOOSE directory with the configure option "
             "'--with-ad-indexing-type=global'");
#endif
  if (!dynamic_cast<INSFVEnergyVariable *>(&_var))
    mooseError("PINSFVEnergyEffectiveDiffusion may only be used with a fluid temperature variable, "
               "of variable type INSFVEnergyVariable.");
}

ADReal
PINSFVEnergyEffectiveDiffusion::computeQpResidual()
{
  // Interpolate thermal conductivity times porosity on the face
  ADRealVectorValue k_eps_face;
  interpolate(Moose::FV::InterpMethod::Average,
              k_eps_face,
              _kappa_elem[_qp] * _eps[_qp],
              _kappa_neighbor[_qp] * _eps_neighbor[_qp],
              *_face_info,
              true);

  // Compute the temperature gradient times the conductivity tensor
  ADRealVectorValue kappa_grad_T;
  const auto grad_T = _var.adGradSln(*_face_info);
  for (std::size_t i = 0; i < LIBMESH_DIM; i++)
    kappa_grad_T(i) = k_eps_face(i) * grad_T(i);

  return -kappa_grad_T * _normal;
}
