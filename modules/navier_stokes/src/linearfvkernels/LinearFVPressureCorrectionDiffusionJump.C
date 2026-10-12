//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVPressureCorrectionDiffusionJump.h"
#include "PorousRhieChowMassFlux.h"

registerMooseObject("NavierStokesApp", LinearFVPressureCorrectionDiffusionJump);

InputParameters
LinearFVPressureCorrectionDiffusionJump::validParams()
{
  InputParameters params = LinearFVPressureCorrectionDiffusion::validParams();
  params.addClassDescription("Pressure correction diffusion kernel that adds a baffle jump "
                             "contribution on internal faces.");
  params.addRequiredParam<UserObjectName>(
      "rhie_chow_user_object", "The rhie-chow user-object which provides baffle jump information.");
  return params;
}

LinearFVPressureCorrectionDiffusionJump::LinearFVPressureCorrectionDiffusionJump(
    const InputParameters & params)
  : LinearFVPressureCorrectionDiffusion(params),
    _rc_uo(getUserObject<PorousRhieChowMassFlux>("rhie_chow_user_object"))
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

  return jumpInterfaceData().transmissibility;
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
  // LinearFVFluxKernel puts +T_f in A(P,P) and -T_f in A(N,P), where those entries multiply the
  // current element pressure p_P.
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
