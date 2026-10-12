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
  _jump_interface_data = _rc_uo.pressureJumpInterfaceData(*_current_face_info,
                                                          _gradient_field.gradient(elem_info),
                                                          _gradient_field.gradient(neighbor_info),
                                                          _current_face_area);

  _jump_interface_data_valid = true;
  return _jump_interface_data;
}

Real
LinearFVPressureCorrectionDiffusionJump::computeJumpAwareFluxMatrixContribution()
{
  const auto & face_operator = jumpInterfaceData();
  return face_operator.is_baffle ? face_operator.transmissibility : computeFluxMatrixContribution();
}

Real
LinearFVPressureCorrectionDiffusionJump::computeJumpAwareFluxRHSContribution()
{
  const auto & face_operator = jumpInterfaceData();
  return face_operator.is_baffle
             ? face_operator.correction + face_operator.transmissibility * face_operator.elem_jump
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
  // J_P is oriented from the opposite side toward P: J_P = p_P - p_N. Let R_f denote the
  // explicit diffusion correction. The two local rows receive
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
  return computeJumpAwareFluxRHSContribution();
}

Real
LinearFVPressureCorrectionDiffusionJump::computeNeighborRightHandSideContribution()
{
  // The neighbor receives the negative of the complete element-side operator, preserving
  // conservation across the internal face.
  return -computeJumpAwareFluxRHSContribution();
}
