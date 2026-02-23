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
      "rhie_chow_user_object",
      "The rhie-chow user-object which provides baffle jump information.");
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
LinearFVAnisotropicDiffusionJump::computeElemRightHandSideContribution()
{
  Real rhs = LinearFVAnisotropicDiffusion::computeElemRightHandSideContribution();

  if (_current_face_info && _current_face_info->neighborPtr())
  {
    const Real jump = _rc_uo.getSignedBaffleJump(*_current_face_info, /*elem_side=*/true);
    if (jump != 0.0)
    {
      rhs += computeFluxMatrixContribution() * jump;
      if (_debug_baffle_jump)
        _console << "Baffle jump RHS (elem) face " << _current_face_info->id()
                 << " jump=" << jump << std::endl;
    }
  }

  return rhs;
}

Real
LinearFVAnisotropicDiffusionJump::computeNeighborRightHandSideContribution()
{
  Real rhs = LinearFVAnisotropicDiffusion::computeNeighborRightHandSideContribution();

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
