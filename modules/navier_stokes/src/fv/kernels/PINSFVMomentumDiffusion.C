//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PINSFVMomentumDiffusion.h"

registerMooseObject("NavierStokesApp", PINSFVMomentumDiffusion);

InputParameters
PINSFVMomentumDiffusion::validParams()
{
  auto params = FVFluxKernel::validParams();
  params.addClassDescription("Viscous diffusion term in the porous media incompressible Navier-Stokes "
                             "momentum equation.");
  params.addRequiredCoupledVar("porosity", "Porosity auxiliary variable");
  params.addRequiredParam<MaterialPropertyName>("mu", "viscosity");
  params.set<unsigned short>("ghost_layers") = 2;
  return params;
}

PINSFVMomentumDiffusion::PINSFVMomentumDiffusion(const InputParameters & params)
  : FVFluxKernel(params),
  _mu_elem(getADMaterialProperty<Real>("mu")),
  _mu_neighbor(getNeighborADMaterialProperty<Real>("mu")),
  _eps(coupledValue("porosity")),
  _eps_neighbor(coupledNeighborValue("porosity")),
  _grad_eps(coupledGradient("porosity"))
{
#ifndef MOOSE_GLOBAL_AD_INDEXING
  mooseError("PINSFV is not supported by local AD indexing. In order to use PINSFV, please run "
             "the configure script in the root MOOSE directory with the configure option "
             "'--with-ad-indexing-type=global'");
#endif
}

ADReal
PINSFVMomentumDiffusion::computeQpResidual()
{
  //TODO: Rewrite with fluid velocity once AuxVariables support AD

  /// Compute the diffusion driven by the velocity gradient
  /// Interpolate viscosity divided by porosity on the face
  ADReal mu_eps_face;
  interpolate(Moose::FV::InterpMethod::Average,
              mu_eps_face,
              _mu_elem[_qp] / _eps[_qp],
              _mu_neighbor[_qp] / _eps_neighbor[_qp],
              *_face_info,
              true);

  /// Compute face superficial velocity gradient
  auto dudn = gradUDotNormal();

  /// First term of residual
  ADReal residual = mu_eps_face * dudn;

  /// Compute the diffusion driven by the porosity gradient
  // /// Interpolate viscosity divided by squared porosity on the face
  // ADReal mu_eps2_face;
  // interpolate(Moose::FV::InterpMethod::Average,
  //             mu_eps2_face,
  //             _mu_elem[_qp] / _eps[_qp] / _eps[_qp],
  //             _mu_neighbor[_qp] / _eps_neighbor[_qp] / _eps_neighbor[_qp],
  //             *_face_info,
  //             true);
  //
  // /// Compute velocity on the face
  //  ADReal v;
  //  this->interpolate(_velocity_interp_method, v, _vel_elem[_qp], _vel_neighbor[_qp]);
  //
  // /// Compute porosity gradient on the face
  // RealVectorValue grad_eps = Moose::FV::gradUDotNormal(_mu_elem[_qp], _mu_neighbor[_qp], *_face_info, _var);
  //
  // /// Add second residual term
  // residual -= mu_eps2_face * v_face * grad_eps;

  return -residual;
}
