//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVPressureCorrectionDiffusionJump.h"
#include "FVReconstructedPressureGradient.h"
#include "MooseUtils.h"
#include "RhieChowMassFlux.h"

registerMooseObject("NavierStokesApp", LinearFVPressureCorrectionDiffusionJump);

InputParameters
LinearFVPressureCorrectionDiffusionJump::validParams()
{
  InputParameters params = LinearFVPressureCorrectionDiffusion::validParams();
  params.addClassDescription("Pressure correction diffusion kernel that adds a baffle jump "
                             "contribution on internal faces.");
  params.addRequiredParam<UserObjectName>(
      "rhie_chow_user_object", "The rhie-chow user-object which provides baffle jump information.");
  params.addParam<bool>(
      "use_two_term_pressure_expansion",
      false,
      "Whether to compute the baffle pressure transmissibility from the lagged two-term cell "
      "pressure expansions.");
  return params;
}

LinearFVPressureCorrectionDiffusionJump::LinearFVPressureCorrectionDiffusionJump(
    const InputParameters & params)
  : LinearFVPressureCorrectionDiffusion(params),
    _rc_uo(getUserObject<RhieChowMassFlux>("rhie_chow_user_object")),
    _reconstructed_pressure_gradient_method(
        dynamic_cast<const FVReconstructedPressureGradient *>(&_gradient_field.method())),
    _use_two_term_pressure_expansion(getParam<bool>("use_two_term_pressure_expansion"))
{
  if (_use_two_term_pressure_expansion && !_reconstructed_pressure_gradient_method)
    paramError("use_two_term_pressure_expansion",
               "Two-term pressure expansion requires FVReconstructedPressureGradient.");
}

Real
LinearFVPressureCorrectionDiffusionJump::computeJumpAwareFluxMatrixContribution()
{
  const Real base_matrix_contribution = computeFluxMatrixContribution();
  if (!_use_two_term_pressure_expansion || !_current_face_info ||
      !_current_face_info->neighborPtr() || !_rc_uo.faceIsBaffle(*_current_face_info) ||
      !_reconstructed_pressure_gradient_method->hasReconstructedCandidate())
    return base_matrix_contribution;

  const auto & elem_info = *_current_face_info->elemInfo();
  const auto & neighbor_info = *_current_face_info->neighborInfo();
  const Point d_elem_face = _current_face_info->faceCentroid() - elem_info.centroid();
  const Point d_neighbor_face = _current_face_info->faceCentroid() - neighbor_info.centroid();
  const auto interp_coeffs =
      Moose::FV::interpCoeffs(Moose::FV::InterpMethod::Average, *_current_face_info, true);

  Real elem_taylor_term = 0.0;
  Real neighbor_taylor_term = 0.0;
  Real two_term_flux = 0.0;
  for (const auto component : make_range(_subproblem.mesh().dimension()))
  {
    const Real elem_gradient = _gradient_field.component(elem_info, component);
    const Real neighbor_gradient = _gradient_field.component(neighbor_info, component);
    elem_taylor_term += elem_gradient * d_elem_face(component);
    neighbor_taylor_term += neighbor_gradient * d_neighbor_face(component);

    const Real elem_coefficient = _rc_uo.cellPressureDiffusionCoefficient(elem_info, component);
    const Real neighbor_coefficient =
        _rc_uo.cellPressureDiffusionCoefficient(neighbor_info, component);
    const Real face_pressure_force =
        interp_coeffs.first * elem_coefficient * elem_gradient +
        interp_coeffs.second * neighbor_coefficient * neighbor_gradient;
    two_term_flux -=
        _current_face_info->normal()(component) * face_pressure_force * _current_face_area;
  }

  const Real two_term_pressure_drop = -elem_taylor_term + neighbor_taylor_term;
  if (MooseUtils::absoluteFuzzyEqual(two_term_pressure_drop, 0.0))
    return base_matrix_contribution;

  const Real two_term_matrix_contribution = two_term_flux / two_term_pressure_drop;
  return std::isfinite(two_term_matrix_contribution) && two_term_matrix_contribution > 0.0
             ? two_term_matrix_contribution
             : base_matrix_contribution;
}

Real
LinearFVPressureCorrectionDiffusionJump::computeElemMatrixContribution()
{
  return computeJumpAwareFluxMatrixContribution();
}

Real
LinearFVPressureCorrectionDiffusionJump::computeNeighborMatrixContribution()
{
  return -computeJumpAwareFluxMatrixContribution();
}

Real
LinearFVPressureCorrectionDiffusionJump::computeElemRightHandSideContribution()
{
  Real rhs = (_current_face_info && _current_face_info->neighborPtr())
                 ? computeFluxRHSContribution()
                 : LinearFVAnisotropicDiffusion::computeElemRightHandSideContribution();

  if (_current_face_info && _current_face_info->neighborPtr())
  {
    const Real jump = _rc_uo.getSignedBaffleJump(*_current_face_info, /*elem_side=*/true);
    if (jump != 0.0)
      rhs += computeJumpAwareFluxMatrixContribution() * jump;
  }

  return rhs;
}

Real
LinearFVPressureCorrectionDiffusionJump::computeNeighborRightHandSideContribution()
{
  Real rhs = (_current_face_info && _current_face_info->neighborPtr())
                 ? -computeFluxRHSContribution()
                 : LinearFVAnisotropicDiffusion::computeNeighborRightHandSideContribution();

  if (_current_face_info && _current_face_info->neighborPtr())
  {
    const Real jump = _rc_uo.getSignedBaffleJump(*_current_face_info, /*elem_side=*/false);
    if (jump != 0.0)
      rhs += computeJumpAwareFluxMatrixContribution() * jump;
  }

  return rhs;
}
