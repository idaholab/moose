//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVEnergyAdvection.h"
#include "MooseLinearVariableFV.h"
#include "NSFVUtils.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", LinearFVEnergyAdvection);

InputParameters
LinearFVEnergyAdvection::validParams()
{
  InputParameters params = LinearFVFluxKernel::validParams();
  params.addClassDescription("Represents the matrix and right hand side contributions of an "
                             "advection term for the energy e.g. h=int(cp dT). A user may still "
                             "override what quantity is advected, but the default is temperature.");
  params.addRequiredParam<Real>("cp", "Specific heat value");
  params.addRequiredParam<UserObjectName>(
      "rhie_chow_user_object",
      "The rhie-chow user-object which is used to determine the face velocity.");
  params += Moose::FV::advectedInterpolationParameter();
  return params;
}

LinearFVEnergyAdvection::LinearFVEnergyAdvection(const InputParameters & params)
  : LinearFVFluxKernel(params),
    _mass_flux_provider(getUserObject<RhieChowMassFlux>("rhie_chow_user_object")),
    _cp(getParam<Real>("cp")),
    _advected_interp_coeffs(std::make_pair<Real, Real>(0, 0)),
    _face_mass_flux(0.0)
{
  Moose::FV::setInterpolationMethod(*this, _advected_interp_method, "advected_interp_method");
}

Real
LinearFVEnergyAdvection::computeElemMatrixContribution()
{
  return _cp * _advected_interp_coeffs.first * _face_mass_flux * _current_face_area;
}

Real
LinearFVEnergyAdvection::computeNeighborMatrixContribution()
{
  return _cp * _advected_interp_coeffs.second * _face_mass_flux * _current_face_area;
}

Real
LinearFVEnergyAdvection::computeElemRightHandSideContribution()
{
  return 0.0;
}

Real
LinearFVEnergyAdvection::computeNeighborRightHandSideContribution()
{
  return 0.0;
}

Real
LinearFVEnergyAdvection::computeBoundaryMatrixContribution(const LinearFVBoundaryCondition & bc)
{
  const auto * const adv_bc = static_cast<const LinearFVAdvectionDiffusionBC *>(&bc);
  mooseAssert(adv_bc, "This should be a valid BC!");

  const auto boundary_value_matrix_contrib = adv_bc->computeBoundaryValueMatrixContribution();

  // We support internal boundaries too so we have to make sure the normal points always outward
  const auto factor = (_current_face_type == FaceInfo::VarFaceNeighbors::ELEM) ? 1.0 : -1.0;

  return _cp * boundary_value_matrix_contrib * factor * _face_mass_flux * _current_face_area;
}

Real
LinearFVEnergyAdvection::computeBoundaryRHSContribution(const LinearFVBoundaryCondition & bc)
{
  const auto * const adv_bc = static_cast<const LinearFVAdvectionDiffusionBC *>(&bc);
  mooseAssert(adv_bc, "This should be a valid BC!");

  // We support internal boundaries too so we have to make sure the normal points always outward
  const auto factor = (_current_face_type == FaceInfo::VarFaceNeighbors::ELEM ? 1.0 : -1.0);

  const auto boundary_value_rhs_contrib = adv_bc->computeBoundaryValueRHSContribution();
  return -_cp * boundary_value_rhs_contrib * factor * _face_mass_flux * _current_face_area;
}

void
LinearFVEnergyAdvection::setupFaceData(const FaceInfo * face_info)
{
  LinearFVFluxKernel::setupFaceData(face_info);

  // Caching the mass flux on the face which will be reused in the advection term's matrix and right
  // hand side contributions
  _face_mass_flux = _mass_flux_provider.getMassFlux(*face_info);

  // Caching the interpolation coefficients so they will be reused for the matrix and right hand
  // side terms
  _advected_interp_coeffs =
      interpCoeffs(_advected_interp_method, *_current_face_info, true, _face_mass_flux);
}
