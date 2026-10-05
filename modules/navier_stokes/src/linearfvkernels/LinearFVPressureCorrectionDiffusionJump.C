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
  // The base coefficient is the usual two-point pressure-diffusion transmissibility. It is also
  // the fallback when the face is not a baffle or a lagged reconstructed gradient is unavailable.
  const Real base_matrix_contribution = computeFluxMatrixContribution();
  if (!_use_two_term_pressure_expansion || !_current_face_info ||
      !_current_face_info->neighborPtr() || !_rc_uo.faceIsBaffle(*_current_face_info) ||
      !_reconstructed_pressure_gradient_method->hasReconstructedCandidate())
    return base_matrix_contribution;

  // Both displacement vectors point from their cell centroid to the common face centroid. The
  // neighbor vector therefore points opposite the P-to-N face orientation on an orthogonal mesh.
  const auto & elem_info = *_current_face_info->elemInfo();
  const auto & neighbor_info = *_current_face_info->neighborInfo();
  const Point d_elem_face = _current_face_info->faceCentroid() - elem_info.centroid();
  const Point d_neighbor_face = _current_face_info->faceCentroid() - neighbor_info.centroid();

  // These are the weights used to interpolate each component of rho/A from the cells to the face.
  const auto interp_coeffs =
      Moose::FV::interpCoeffs(Moose::FV::InterpMethod::Average, *_current_face_info, true);

  // Everything used below is fixed while the current pressure-correction system is assembled.
  // In particular, _gradient_field contains the coupling gradient published after the preceding
  // pressure corrector; the Taylor terms are not functions of the pressure unknowns in this solve.
  // They are used only to construct a lagged (Picard) coefficient T_f^lag, which will multiply the
  // current cell pressures in the matrix.
  //
  // Let P and N denote the element and neighbor cells, respectively, and define
  // d_Pf = x_f - x_P and d_Nf = x_f - x_N. The Taylor expansions to the two one-sided face
  // pressures are
  //
  //   p_f^P = p_P + grad(p)_P . d_Pf,
  //   p_f^N = p_N + grad(p)_N . d_Nf.
  //
  // With the element-oriented jump J_P = p_f^P - p_f^N, eliminating the two face pressures gives
  // the lagged pressure drop after removing the modeled discontinuity:
  //
  //   Delta p_PN^smooth = (p_P - p_N) - J_P
  //                      = -grad(p)_P . d_Pf + grad(p)_N . d_Nf.
  Real elem_taylor_term = 0.0;
  Real neighbor_taylor_term = 0.0;
  Real two_term_flux = 0.0;
  for (const auto component : make_range(_subproblem.mesh().dimension()))
  {
    // The reader supplies already-published scalar gradient components. These Real values remain
    // constant throughout assembly and solution of the current pressure system.
    const Real elem_gradient = _gradient_field.component(elem_info, component);
    const Real neighbor_gradient = _gradient_field.component(neighbor_info, component);

    // Accumulate grad(p)_P . d_Pf and grad(p)_N . d_Nf. Their signed difference below is the
    // lagged smooth pressure drop used only to calculate T_f^lag.
    elem_taylor_term += elem_gradient * d_elem_face(component);
    neighbor_taylor_term += neighbor_gradient * d_neighbor_face(component);

    // rho/A is also known from the completed momentum predictor, so it contributes no pressure
    // unknowns to the matrix.
    const Real elem_coefficient = _rc_uo.cellPressureDiffusionCoefficient(elem_info, component);
    const Real neighbor_coefficient =
        _rc_uo.cellPressureDiffusionCoefficient(neighbor_info, component);

    // Interpolate the lagged coefficient-gradient product for this Cartesian component.
    const Real face_pressure_force =
        interp_coeffs.first * elem_coefficient * elem_gradient +
        interp_coeffs.second * neighbor_coefficient * neighbor_gradient;

    // With D_i = rho (A_i)^-1, the corresponding pressure-gradient flux is
    //
    //   q_p = -A_f sum_i n_i (D_i dp/dx_i)_f,
    //
    // where each coefficient-gradient product uses the geometric face interpolation above.
    two_term_flux -=
        _current_face_info->normal()(component) * face_pressure_force * _current_face_area;
  }

  // This is Delta p_PN^smooth,lag. It is a known denominator, not a pressure difference assembled
  // from the current solution vector.
  const Real two_term_pressure_drop = -elem_taylor_term + neighbor_taylor_term;

  // A vanishing lagged pressure drop cannot define a transmissibility, even if roundoff leaves a
  // nonzero lagged flux. Retain the ordinary pressure-diffusion matrix coefficient in that case.
  if (MooseUtils::absoluteFuzzyEqual(two_term_pressure_drop, 0.0))
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
  const Real two_term_matrix_contribution =
      (two_term_flux + computeFluxRHSContribution()) / two_term_pressure_drop;

  // A negative or non-finite quotient would not define a diffusive matrix stencil.
  return std::isfinite(two_term_matrix_contribution) && two_term_matrix_contribution > 0.0
             ? two_term_matrix_contribution
             : base_matrix_contribution;
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
  // Internal faces use the explicit nonorthogonal part of the base diffusion operator. Boundary
  // faces retain the complete base boundary treatment.
  Real rhs = (_current_face_info && _current_face_info->neighborPtr())
                 ? computeFluxRHSContribution()
                 : LinearFVAnisotropicDiffusion::computeElemRightHandSideContribution();

  if (_current_face_info && _current_face_info->neighborPtr())
  {
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
  }

  return rhs;
}

Real
LinearFVPressureCorrectionDiffusionJump::computeNeighborRightHandSideContribution()
{
  // The neighbor receives the opposite base flux and the oppositely oriented jump
  // J_N = p_N - p_P = -J_P, preserving conservation across the internal face.
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
