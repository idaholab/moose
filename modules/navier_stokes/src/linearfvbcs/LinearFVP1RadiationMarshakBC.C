//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVP1RadiationMarshakBC.h"

registerMooseObject("NavierStokesApp", LinearFVP1RadiationMarshakBC);

InputParameters
LinearFVP1RadiationMarshakBC::validParams()
{
  InputParameters params = LinearFVAdvectionDiffusionBC::validParams();
  params.addClassDescription("Marshak boundary condition for radiative heat flux.");
  params.addRequiredParam<MooseFunctorName>("temperature_radiation", "The radiation temperature.");
  params.addRequiredParam<MooseFunctorName>("coeff_diffusion",
                                            "Radiative heat flux P1 diffusion coefficient.");
  params.addParam<Real>("boundary_emissivity", 1.0, "Emissivity of the boundary.");
  return params;
}

LinearFVP1RadiationMarshakBC::LinearFVP1RadiationMarshakBC(
    const InputParameters & parameters)
  : LinearFVAdvectionDiffusionBC(parameters),
    _temperature_radiation(getFunctor<Real>("temperature_radiation")),
    _coeff_diffusion(getFunctor<Real>("coeff_diffusion")),
    _eps_boundary(getParam<Real>("boundary_emissivity"))
{
  _var.computeCellGradients();
}

Real
LinearFVP1RadiationMarshakBC::computeBoundaryValue() const
{
  const auto face = singleSidedFaceArg(_current_face_info);
  mooseAssert(_current_face_type != FaceInfo::VarFaceNeighbors::BOTH,
              "This should not be assigned on an internal face!");
  const auto & elem_info = _current_face_type == FaceInfo::VarFaceNeighbors::ELEM
                               ? _current_face_info->elemInfo()
                               : _current_face_info->neighborInfo();
  const auto state = determineState();

  const auto alpha = -_coeff_diffusion(face, state);
  const auto beta = -_eps_boundary / (2 * (2 - _eps_boundary));
  const auto gamma = -_eps_boundary * 4 * HeatConduction::Constants::sigma *
      Utility::pow<4>(_temperature_radiation(face,state)) /  ( (2 * (2 - _eps_boundary)) );

  const auto phi = _var.getElemValue(*elem_info, state);
  const auto grad_phi = _var.gradSln(*elem_info);

  const auto & nhat = _current_face_info->normal();

  const auto d_cf = computeCellToFaceVector(); // vector from boundary cell centre to boundary face
  const auto projection = d_cf * nhat;
  const auto vc = d_cf - (projection * nhat);
  return ((alpha * phi) + (alpha * grad_phi * vc) + (gamma * projection)) /
         (alpha + (beta * projection));
}

Real
LinearFVP1RadiationMarshakBC::computeBoundaryNormalGradient() const
{
  const auto face = singleSidedFaceArg(_current_face_info);
  const auto state = determineState();
  const auto alpha = -_coeff_diffusion(face, state);
  mooseAssert(!MooseUtils::isZero(alpha), "Alpha should not be 0!");
  const auto beta = -_eps_boundary / (2 * (2 - _eps_boundary));
  const auto gamma = -_eps_boundary * 4 * HeatConduction::Constants::sigma *
      Utility::pow<4>(_temperature_radiation(face,state)) /  ( (2 * (2 - _eps_boundary)) );

  return (gamma - beta * computeBoundaryValue()) / alpha;
}

// implicit terms for advection kernel
Real
LinearFVP1RadiationMarshakBC::computeBoundaryValueMatrixContribution() const
{
  const auto face = singleSidedFaceArg(_current_face_info);
  const auto state = determineState();
  const auto alpha = -_coeff_diffusion(face, state);
  const auto beta = -_eps_boundary / (2 * (2 - _eps_boundary));
  const auto & nhat = _current_face_info->normal();

  return alpha / (alpha + (beta * computeCellToFaceVector() * nhat));
}

// explicit terms for advection kernel
Real
LinearFVP1RadiationMarshakBC::computeBoundaryValueRHSContribution() const
{
  const auto face = singleSidedFaceArg(_current_face_info);
  const auto state = determineState();
  mooseAssert(_current_face_type != FaceInfo::VarFaceNeighbors::BOTH,
              "This should not be assigned on an internal face!");
  const auto & elem_info = _current_face_type == FaceInfo::VarFaceNeighbors::ELEM
                               ? _current_face_info->elemInfo()
                               : _current_face_info->neighborInfo();
  const auto alpha = -_coeff_diffusion(face, state);
  const auto beta = -_eps_boundary / (2 * (2 - _eps_boundary));
  const auto gamma = -_eps_boundary * 4 * HeatConduction::Constants::sigma *
      Utility::pow<4>(_temperature_radiation(face,state)) /  ( (2 * (2 - _eps_boundary)) );
  const auto & grad_phi = _var.gradSln(*elem_info);

  const auto & nhat = _current_face_info->normal();

  const auto d_cf = computeCellToFaceVector(); // vector from boundary cell centre to boundary face
  const auto projection = d_cf * nhat;
  const auto vc = d_cf - (projection * nhat); // correction vector for non-orthogonal cells

  return (gamma * projection / (alpha + (beta * projection))) +
         (alpha * grad_phi * vc / (alpha + (beta * projection)));
}

// implicit terms for diffusion kernel
Real
LinearFVP1RadiationMarshakBC::computeBoundaryGradientMatrixContribution() const
{
  const auto face = singleSidedFaceArg(_current_face_info);
  const auto state = determineState();

  const auto alpha = -_coeff_diffusion(face, state);
  const auto beta = -_eps_boundary / (2 * (2 - _eps_boundary));
  const auto & nhat = _current_face_info->normal();

  return beta / (alpha + (beta * computeCellToFaceVector() * nhat));
}

// explicit terms for diffusion kernel
Real
LinearFVP1RadiationMarshakBC::computeBoundaryGradientRHSContribution() const
{
  mooseAssert(_current_face_type != FaceInfo::VarFaceNeighbors::BOTH,
              "This should not be assigned on an internal face!");
  const auto & elem_info = _current_face_type == FaceInfo::VarFaceNeighbors::ELEM
                               ? _current_face_info->elemInfo()
                               : _current_face_info->neighborInfo();
  const auto face = singleSidedFaceArg(_current_face_info);
  const auto state = determineState();
  const auto & grad_phi = _var.gradSln(*elem_info);

  const auto alpha = -_coeff_diffusion(face, state);
  const auto beta = -_eps_boundary / (2 * (2 - _eps_boundary));
  const auto gamma = -_eps_boundary * 4 * HeatConduction::Constants::sigma *
      Utility::pow<4>(_temperature_radiation(face,state)) /  ( (2 * (2 - _eps_boundary)) );

  const auto & nhat = _current_face_info->normal();

  const auto d_cf = computeCellToFaceVector(); // vector from boundary cell centre to boundary face
  const auto projection = d_cf * nhat;
  const auto vc = d_cf - (projection * nhat); // correction vector for non-orthogonal cells

  return (gamma / alpha) + (-beta * gamma * projection / alpha / (alpha + (beta * projection))) +
         (-beta * grad_phi * vc / (alpha + (beta * projection)));
}
