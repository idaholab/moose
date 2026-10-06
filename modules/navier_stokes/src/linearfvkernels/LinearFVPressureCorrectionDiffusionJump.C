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
#include "FVUtils.h"
#include "RhieChowMassFlux.h"

#include <limits>

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
    _use_two_term_pressure_expansion(getParam<bool>("use_two_term_pressure_expansion"))
{
}

void
LinearFVPressureCorrectionDiffusionJump::setupFaceData(const FaceInfo * face_info)
{
  LinearFVPressureCorrectionDiffusion::setupFaceData(face_info);
  _jump_interface_data_valid = false;
}

const NS::FV::PressureJumpInterfaceData &
LinearFVPressureCorrectionDiffusionJump::jumpInterfaceData()
{
  if (_jump_interface_data_valid)
    return _jump_interface_data;

  mooseAssert(_current_face_info && _current_face_info->neighborPtr(),
              "Jump-interface data requires an internal face.");

  const auto & elem_info = *_current_face_info->elemInfo();
  const auto & neighbor_info = *_current_face_info->neighborInfo();
  const auto elem_gradient = _gradient_field.gradient(elem_info);
  const auto neighbor_gradient = _gradient_field.gradient(neighbor_info);

  RealVectorValue elem_diffusion;
  RealVectorValue neighbor_diffusion;
  for (const auto component : make_range(_subproblem.mesh().dimension()))
  {
    elem_diffusion(component) = _rc_uo.cellPressureDiffusionCoefficient(elem_info, component);
    neighbor_diffusion(component) =
        _rc_uo.cellPressureDiffusionCoefficient(neighbor_info, component);
  }
  const Point elem_to_face = _current_face_info->faceCentroid() - elem_info.centroid();
  const Point face_to_neighbor = neighbor_info.centroid() - _current_face_info->faceCentroid();

  _jump_interface_data = NS::FV::pressureJumpInterfaceData(_current_face_info->normal(),
                                                           elem_to_face,
                                                           face_to_neighbor,
                                                           elem_diffusion,
                                                           neighbor_diffusion,
                                                           elem_gradient,
                                                           neighbor_gradient,
                                                           _current_face_area,
                                                           _use_nonorthogonal_correction);

  if (!_jump_interface_data.valid)
  {
    // Preserve one common conservative coefficient if a degenerate half-cell geometry or
    // coefficient prevents elimination of the two interface pressures. No one-sided correction
    // can be formed safely in this case.
    _jump_interface_data.transmissibility = computeFluxMatrixContribution();
    _jump_interface_data.correction = 0.0;
    _jump_interface_data.valid = true;
  }

  _jump_interface_data_valid = true;
  return _jump_interface_data;
}

Real
LinearFVPressureCorrectionDiffusionJump::computeJumpAwareFluxMatrixContribution()
{
  const Real base_matrix_contribution = computeFluxMatrixContribution();
  if (!_rc_uo.faceIsBaffle(*_current_face_info))
    return base_matrix_contribution;

  if (!_use_two_term_pressure_expansion)
    return base_matrix_contribution;

  const auto & reconstructed_gradient_field = _rc_uo.pressureGradientField();
  const auto * const reconstructed_gradient_method =
      dynamic_cast<const FVReconstructedPressureGradient *>(&reconstructed_gradient_field.method());
  if (!reconstructed_gradient_method)
    paramError("use_two_term_pressure_expansion",
               "Two-term pressure expansion requires the momentum-pressure kernels linked to "
               "the Rhie-Chow object to use FVReconstructedPressureGradient.");
  if (!reconstructed_gradient_method->hasReconstructedCandidate())
    return base_matrix_contribution;

  const auto & interface_data = jumpInterfaceData();
  const auto & elem_info = *_current_face_info->elemInfo();
  const auto & neighbor_info = *_current_face_info->neighborInfo();
  const Point elem_to_face = _current_face_info->faceCentroid() - elem_info.centroid();
  const Point neighbor_to_face = _current_face_info->faceCentroid() - neighbor_info.centroid();
  Real elem_taylor_term = 0.0;
  Real neighbor_taylor_term = 0.0;
  for (const auto component : make_range(_subproblem.mesh().dimension()))
  {
    elem_taylor_term +=
        reconstructed_gradient_field.component(elem_info, component) * elem_to_face(component);
    neighbor_taylor_term += reconstructed_gradient_field.component(neighbor_info, component) *
                            neighbor_to_face(component);
  }

  // This is Delta p_PN^smooth,lag. It is a known denominator, not a pressure difference assembled
  // from the current solution vector.
  const Real two_term_pressure_drop = -elem_taylor_term + neighbor_taylor_term;

  // A vanishing lagged pressure drop cannot define a transmissibility, even if roundoff leaves a
  // nonzero lagged flux. Retain the ordinary pressure-diffusion matrix coefficient in that case.
  // Thirty-two floating-point ulps leave headroom for the two dot-product accumulations while
  // scaling the cancellation test with the actual Taylor terms instead of a dimensional constant.
  const Real cancellation_scale = std::abs(elem_taylor_term) + std::abs(neighbor_taylor_term);
  if (two_term_pressure_drop == 0.0 ||
      std::abs(two_term_pressure_drop) <=
          32.0 * std::numeric_limits<Real>::epsilon() * cancellation_scale)
    return base_matrix_contribution;

  // Both q_p^lag and Delta p_PN^smooth,lag are known numbers here. The assembled face relation is
  //
  //   q_p = T_f Delta p_PN^smooth - R_f,
  //
  // so matching the reconstructed flux requires
  //
  //   T_f^lag = (q_p^lag + R_f^lag) / Delta p_PN^smooth,lag.
  //
  // The face area is already included in both flux terms, so T_f^lag is the complete
  // area-integrated matrix coefficient. A valid pressure-diffusion transmissibility is positive.
  // Degenerate or inconsistent reconstructed data revert to the base discretization.
  // The quotient is a frozen scalar. Returning it through compute*MatrixContribution() makes it a
  // coefficient of the current p_P and p_N unknowns; neither lagged gradient enters the matrix as
  // an unknown.
  const auto interpolation_weights =
      Moose::FV::interpCoeffs(Moose::FV::InterpMethod::Average, *_current_face_info, true);
  Real two_term_flux = 0.0;
  for (const auto component : make_range(_subproblem.mesh().dimension()))
  {
    const Real elem_pressure_force = _rc_uo.cellPressureDiffusionCoefficient(elem_info, component) *
                                     reconstructed_gradient_field.component(elem_info, component);
    const Real neighbor_pressure_force =
        _rc_uo.cellPressureDiffusionCoefficient(neighbor_info, component) *
        reconstructed_gradient_field.component(neighbor_info, component);
    two_term_flux -= _current_face_info->normal()(component) * _current_face_area *
                     (interpolation_weights.first * elem_pressure_force +
                      interpolation_weights.second * neighbor_pressure_force);
  }
  const Real two_term_matrix_contribution =
      (two_term_flux + interface_data.correction) / two_term_pressure_drop;

  // A negative or non-finite quotient would not define a diffusive matrix stencil.
  return std::isfinite(two_term_matrix_contribution) && two_term_matrix_contribution > 0.0
             ? two_term_matrix_contribution
             : base_matrix_contribution;
}

Real
LinearFVPressureCorrectionDiffusionJump::computeJumpAwareFluxRHSContribution()
{
  return _rc_uo.faceIsBaffle(*_current_face_info) ? jumpInterfaceData().correction
                                                  : computeFluxRHSContribution();
}

Real
LinearFVPressureCorrectionDiffusionJump::computeElemMatrixContribution()
{
  // The gradients determine a frozen T_f; LinearFVFluxKernel puts +T_f in A(P,P) and -T_f in
  // A(N,P), where those entries multiply the current element pressure p_P.
  return computeJumpAwareFluxMatrixContribution();
}

Real
LinearFVPressureCorrectionDiffusionJump::computeNeighborMatrixContribution()
{
  // Returning -T_f puts -T_f in A(P,N) and +T_f in A(N,N), where those entries multiply the
  // current neighbor pressure p_N.
  return -computeJumpAwareFluxMatrixContribution();
}

Real
LinearFVPressureCorrectionDiffusionJump::computeElemRightHandSideContribution()
{
  Real rhs = computeJumpAwareFluxRHSContribution();

  // J_P is oriented from the opposite side toward P: J_P = p_P - p_N. Let R_f denote the
  // explicit diffusion correction returned above. The two local rows receive
  //
  //   b_P =  R_f + T_f J_P,
  //   b_N = -R_f + T_f J_N = -R_f - T_f J_P.
  //
  // Combined with the local matrix documented above, the element row is
  //
  //   T_f (p_P - p_N) = R_f + T_f J_P,
  //
  // or T_f [(p_P - p_N) - J_P] = R_f. The neighbor equation is its negative, so the modeled jump
  // changes only the right hand side and the face contribution remains conservative.
  const Real jump = _rc_uo.getSignedBaffleJump(*_current_face_info, /*elem_side=*/true);
  if (jump != 0.0)
    rhs += computeJumpAwareFluxMatrixContribution() * jump;

  return rhs;
}

Real
LinearFVPressureCorrectionDiffusionJump::computeNeighborRightHandSideContribution()
{
  // The neighbor receives the opposite base flux and the oppositely oriented jump
  // J_N = p_N - p_P = -J_P, preserving conservation across the internal face.
  Real rhs = -computeJumpAwareFluxRHSContribution();

  const Real jump = _rc_uo.getSignedBaffleJump(*_current_face_info, /*elem_side=*/false);
  if (jump != 0.0)
    rhs += computeJumpAwareFluxMatrixContribution() * jump;

  return rhs;
}
