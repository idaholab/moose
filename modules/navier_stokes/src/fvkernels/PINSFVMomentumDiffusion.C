//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PINSFVMomentumDiffusion.h"
#include "PINSFVSuperficialVelocityVariable.h"
#include "NS.h"
#include "INSFVRhieChowInterpolator.h"
#include "SystemBase.h"

registerMooseObject("NavierStokesApp", PINSFVMomentumDiffusion);

InputParameters
PINSFVMomentumDiffusion::validParams()
{
  auto params = INSFVMomentumDiffusion::validParams();
  params.addClassDescription(
      "Viscous diffusion term, div(mu eps grad(u_d / eps)), in the porous media "
      "incompressible Navier-Stokes momentum equation.");
  params.addRequiredParam<MooseFunctorName>(NS::porosity, "Porosity auxiliary variable");
  return params;
}

PINSFVMomentumDiffusion::PINSFVMomentumDiffusion(const InputParameters & params)
  : INSFVMomentumDiffusion(params), _eps(getFunctor<ADReal>(NS::porosity))
{
#ifndef MOOSE_GLOBAL_AD_INDEXING
  mooseError("PINSFV is not supported by local AD indexing. In order to use PINSFV, please run "
             "the configure script in the root MOOSE directory with the configure option "
             "'--with-ad-indexing-type=global'");
#endif
  if (!dynamic_cast<PINSFVSuperficialVelocityVariable *>(&_var))
    mooseError("PINSFVMomentumDiffusion may only be used with a superficial velocity "
               "variable, of variable type PINSFVSuperficialVelocityVariable.");
}

ADReal
PINSFVMomentumDiffusion::computeStrongResidual()
{
#ifdef MOOSE_GLOBAL_AD_INDEXING
  using namespace Moose::FV;

  const auto elem_face = elemFromFace();
  const auto neighbor_face = neighborFromFace();
  const auto mu_elem = _mu(elem_face);
  const auto mu_neighbor = _mu(neighbor_face);
  const auto eps_elem = _eps(elem_face);
  const auto eps_neighbor = _eps(neighbor_face);

  // Compute the diffusion driven by the velocity gradient
  // Interpolate viscosity divided by porosity on the face
  ADReal mu_face;
  interpolate(Moose::FV::InterpMethod::Average, mu_face, mu_elem, mu_neighbor, *_face_info, true);

  // Compute face superficial velocity gradient
  auto dudn =
      _var.gradient(Moose::FV::makeCDFace(*_face_info, faceArgSubdomains())) * _face_info->normal();

  if (_face_type == FaceInfo::VarFaceNeighbors::ELEM ||
      _face_type == FaceInfo::VarFaceNeighbors::BOTH)
  {
    const auto dof_number = _face_info->elem().dof_number(_sys.number(), _var.number(), 0);
    // A gradient is a linear combination of degrees of freedom so it's safe to straight-up index
    // into the derivatives vector at the dof we care about
    _ae = dudn.derivatives()[dof_number];
    _ae *= -mu_face;
  }
  if (_face_type == FaceInfo::VarFaceNeighbors::NEIGHBOR ||
      _face_type == FaceInfo::VarFaceNeighbors::BOTH)
  {
    const auto dof_number = _face_info->neighbor().dof_number(_sys.number(), _var.number(), 0);
    _an = dudn.derivatives()[dof_number];
    _an *= mu_face;
  }

  // First term of residual
  ADReal residual = mu_face * dudn;

  // Get the face porosity gradient separately
  const auto & grad_eps_face =
      MetaPhysicL::raw_value(_eps.gradient(Moose::FV::makeCDFace(*_face_info)));

  const auto coeff_elem = mu_elem / eps_elem * _var(elem_face);
  const auto coeff_neighbor = mu_neighbor / eps_neighbor * _var(neighbor_face);

  // Interpolate to get the face value
  ADReal coeff_face;
  interpolate(
      Moose::FV::InterpMethod::Average, coeff_face, coeff_elem, coeff_neighbor, *_face_info, true);
  residual -= coeff_face * grad_eps_face * _normal;

  return -residual;
#else
  return 0;
#endif
}
