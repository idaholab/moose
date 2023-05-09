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
  if (!dynamic_cast<PINSFVSuperficialVelocityVariable *>(&_var))
    mooseError("PINSFVMomentumDiffusion may only be used with a superficial velocity "
               "variable, of variable type PINSFVSuperficialVelocityVariable.");
}

ADReal
PINSFVMomentumDiffusion::computeStrongResidual()
{
  using namespace Moose::FV;

  const bool has_elem = (_face_type == FaceInfo::VarFaceNeighbors::ELEM ||
                         _face_type == FaceInfo::VarFaceNeighbors::BOTH);
  const bool has_neighbor = (_face_type == FaceInfo::VarFaceNeighbors::NEIGHBOR ||
                             _face_type == FaceInfo::VarFaceNeighbors::BOTH);

  const auto elem_face = elemArg();
  const auto neighbor_face = neighborArg();
  const auto state = determineState();

  // Compute the diffusion driven by the velocity gradient
  // Interpolate viscosity divided by porosity on the face
  ADReal mu_face;

  const auto mu_elem = has_elem ? _mu(elem_face, state) : _mu(neighbor_face, state);
  const auto eps_elem = has_elem ? _eps(elem_face, state) : _eps(neighbor_face, state);

  ADReal mu_neighbor;
  ADReal eps_neighbor;
  if (onBoundary(*_face_info))
    mu_face = _mu(singleSidedFaceArg(), state);
  else
  {
    mu_neighbor = has_elem ? _mu(neighbor_face, state) : _mu(elem_face, state);
    eps_neighbor = has_neighbor ? _eps(neighbor_face, state) : _eps(elem_face, state);
    interpolate(Moose::FV::InterpMethod::Average, mu_face, mu_elem, mu_neighbor, *_face_info, true);
  }

  // Compute face superficial velocity gradient
  auto dudn = _var.gradient(makeCDFace(*_face_info), state) * _face_info->normal();

  if (has_elem)
  {
    const auto dof_number = _face_info->elem().dof_number(_sys.number(), _var.number(), 0);
    // A gradient is a linear combination of degrees of freedom so it's safe to straight-up index
    // into the derivatives vector at the dof we care about
    _ae = dudn.derivatives()[dof_number];
    _ae *= -mu_face;
  }
  if (has_neighbor)
  {
    const auto dof_number = _face_info->neighbor().dof_number(_sys.number(), _var.number(), 0);
    _an = dudn.derivatives()[dof_number];
    _an *= mu_face;
  }

  // First term of residual
  ADReal residual = mu_face * dudn;

  // Get the face porosity gradient separately
  const auto & grad_eps_face =
      (has_elem && has_neighbor)
          ? MetaPhysicL::raw_value(_eps.gradient(makeCDFace(*_face_info), state))
          : MetaPhysicL::raw_value(_eps.gradient(
                makeElemArg(has_elem ? &_face_info->elem() : _face_info->neighborPtr()), state));

  // Interpolate to get the face value
  ADReal coeff_face;
  // At this point, we already computed mu_elem/eps_elem by knowing which element owns the
  // face, so it is enough to switch between the variable evaluation here
  const auto coeff_one_side =
      mu_elem / eps_elem * (has_elem ? _var(elem_face, state) : _var(neighbor_face, state));
  if (onBoundary(*_face_info))
    coeff_face = coeff_one_side;
  else
  {
    mooseAssert(has_elem, "We should be defined on the element side if we're not on a boundary");
    const auto coeff_neighbor = mu_neighbor / eps_neighbor *
                                (has_elem ? _var(neighbor_face, state) : _var(elem_face, state));
    interpolate(Moose::FV::InterpMethod::Average,
                coeff_face,
                coeff_one_side,
                coeff_neighbor,
                *_face_info,
                true);
  }

  residual -= coeff_face * grad_eps_face * _normal;

  return -residual;
}
