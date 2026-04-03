//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVAnisotropicDiffusionJump.h"
#include "RhieChowMassFlux.h"

registerMooseObject("NavierStokesApp", LinearFVAnisotropicDiffusionJump);

InputParameters
LinearFVAnisotropicDiffusionJump::validParams()
{
  InputParameters params = LinearFVAnisotropicDiffusion::validParams();
  params.addClassDescription(
      "Anisotropic diffusion kernel that adds a baffle jump contribution on internal faces.");
  params.addRequiredParam<UserObjectName>(
      "rhie_chow_user_object", "The rhie-chow user-object which provides baffle jump information.");
  params.addParam<bool>("debug_baffle_jump", false, "Enable debug output for baffle jumps.");
  return params;
}

LinearFVAnisotropicDiffusionJump::LinearFVAnisotropicDiffusionJump(const InputParameters & params)
  : LinearFVAnisotropicDiffusion(params),
    _rc_uo(getUserObject<RhieChowMassFlux>("rhie_chow_user_object")),
    _debug_baffle_jump(getParam<bool>("debug_baffle_jump"))
{
}

Real
LinearFVAnisotropicDiffusionJump::computeJumpAwareInternalFluxRHSContribution()
{
  if (!_cached_rhs_contribution)
  {
    mooseAssert(_current_face_info && _current_face_info->elemInfo() &&
                    _current_face_info->neighborInfo(),
                "Jump-aware internal flux RHS requires both element and neighbor data.");

    const auto face_arg = makeCDFace(*_current_face_info);
    const auto state_arg = determineState();

    RealVectorValue grad_elem(0.0);
    RealVectorValue grad_neighbor(0.0);
    for (const auto i : make_range(Moose::dim))
    {
      grad_elem(i) = _rc_uo.pressureGradient(*_current_face_info->elemInfo(), i);
      grad_neighbor(i) = _rc_uo.pressureGradient(*_current_face_info->neighborInfo(), i);
    }

    const auto avg_interp_coeffs =
        Moose::FV::interpCoeffs(Moose::FV::InterpMethod::Average, *_current_face_info, true);
    const auto averaged_gradient =
        avg_interp_coeffs.first * grad_elem + avg_interp_coeffs.second * grad_neighbor;

    auto scaled_diff_tensor = _diffusion_tensor(face_arg, state_arg);

    for (const auto i : make_range(Moose::dim))
      scaled_diff_tensor(i) = _current_face_info->normal()(i) * scaled_diff_tensor(i);

    const auto normal_scaled_diff_tensor = scaled_diff_tensor * _current_face_info->normal();

    _flux_rhs_contribution =
        (scaled_diff_tensor - normal_scaled_diff_tensor * _current_face_info->normal()) *
        averaged_gradient;

    if (_use_nonorthogonal_correction)
    {
      const auto correction_vector =
          _current_face_info->normal() -
          1 / (_current_face_info->normal() * _current_face_info->eCN()) *
              _current_face_info->eCN();

      _flux_rhs_contribution += normal_scaled_diff_tensor * averaged_gradient * correction_vector;
    }

    _flux_rhs_contribution *= _current_face_area;
    _cached_rhs_contribution = true;
  }

  return _flux_rhs_contribution;
}

Real
LinearFVAnisotropicDiffusionJump::computeElemRightHandSideContribution()
{
  Real rhs = (_current_face_info && _current_face_info->neighborPtr())
                 ? computeJumpAwareInternalFluxRHSContribution()
                 : LinearFVAnisotropicDiffusion::computeElemRightHandSideContribution();

  if (_current_face_info && _current_face_info->neighborPtr())
  {
    const Real jump = _rc_uo.getSignedBaffleJump(*_current_face_info, /*elem_side=*/true);
    if (jump != 0.0)
    {
      rhs += computeFluxMatrixContribution() * jump;
      if (_debug_baffle_jump)
        _console << "Baffle jump RHS (elem) face " << _current_face_info->id() << " jump=" << jump
                 << std::endl;
    }
  }

  return rhs;
}

Real
LinearFVAnisotropicDiffusionJump::computeNeighborRightHandSideContribution()
{
  Real rhs = (_current_face_info && _current_face_info->neighborPtr())
                 ? -computeJumpAwareInternalFluxRHSContribution()
                 : LinearFVAnisotropicDiffusion::computeNeighborRightHandSideContribution();

  if (_current_face_info && _current_face_info->neighborPtr())
  {
    const Real jump = _rc_uo.getSignedBaffleJump(*_current_face_info, /*elem_side=*/false);
    if (jump != 0.0)
    {
      rhs += computeFluxMatrixContribution() * jump;
      if (_debug_baffle_jump)
        _console << "Baffle jump RHS (neighbor) face " << _current_face_info->id()
                 << " jump=" << jump << std::endl;
    }
  }

  return rhs;
}
