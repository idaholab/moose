//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVScalarAdvection.h"
#include "MooseLinearVariableFV.h"
#include "NSFVUtils.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", LinearFVScalarAdvection);

InputParameters
LinearFVScalarAdvection::validParams()
{
  InputParameters params = LinearFVFluxKernel::validParams();
  params.addClassDescription("Represents the matrix and right hand side contributions of an "
                             "advection term for a passive scalar.");
  params.addRequiredParam<UserObjectName>(
      "rhie_chow_user_object",
      "The rhie-chow user-object which is used to determine the face velocity.");
  params += Moose::FV::advectedInterpolationParameter();
  return params;
}

LinearFVScalarAdvection::LinearFVScalarAdvection(const InputParameters & params)
  : LinearFVFluxKernel(params),
    _mass_flux_provider(getUserObject<RhieChowMassFlux>("rhie_chow_user_object")),
    _advected_interp_coeffs(std::make_pair<Real, Real>(0, 0)),
    _volumetric_face_flux(0.0)
{
  Moose::FV::setInterpolationMethod(*this, _advected_interp_method, "advected_interp_method");
}

Real
LinearFVScalarAdvection::computeElemMatrixContribution()
{
  return _advected_interp_coeffs.first * _volumetric_face_flux * _current_face_area;
}

Real
LinearFVScalarAdvection::computeNeighborMatrixContribution()
{
  return _advected_interp_coeffs.second * _volumetric_face_flux * _current_face_area;
}

Real
LinearFVScalarAdvection::computeElemRightHandSideContribution()
{
  return 0.0;
}

Real
LinearFVScalarAdvection::computeNeighborRightHandSideContribution()
{
  return 0.0;
}

Real
LinearFVScalarAdvection::computeBoundaryMatrixContribution(const LinearFVBoundaryCondition & bc)
{
  const auto * const adv_bc = static_cast<const LinearFVAdvectionDiffusionBC *>(&bc);
  mooseAssert(adv_bc, "This should be a valid BC!");

  const auto boundary_value_matrix_contrib = adv_bc->computeBoundaryValueMatrixContribution();

  // We support internal boundaries too so we have to make sure the normal points always outward
  const auto factor = (_current_face_type == FaceInfo::VarFaceNeighbors::ELEM) ? 1.0 : -1.0;

  return boundary_value_matrix_contrib * factor * _volumetric_face_flux * _current_face_area;
}

Real
LinearFVScalarAdvection::computeBoundaryRHSContribution(const LinearFVBoundaryCondition & bc)
{
  const auto * const adv_bc = static_cast<const LinearFVAdvectionDiffusionBC *>(&bc);
  mooseAssert(adv_bc, "This should be a valid BC!");

  // We support internal boundaries too so we have to make sure the normal points always outward
  const auto factor = (_current_face_type == FaceInfo::VarFaceNeighbors::ELEM ? 1.0 : -1.0);

  const auto boundary_value_rhs_contrib = adv_bc->computeBoundaryValueRHSContribution();
  return -boundary_value_rhs_contrib * factor * _volumetric_face_flux * _current_face_area;
}

void
LinearFVScalarAdvection::setupFaceData(const FaceInfo * face_info)
{
  LinearFVFluxKernel::setupFaceData(face_info);

  // Caching the velocity on the face which will be reused in the advection term's matrix and right
  // hand side contributions
  _volumetric_face_flux = _mass_flux_provider.getVolumetricFaceFlux(*face_info);

  // Caching the interpolation coefficients so they will be reused for the matrix and right hand
  // side terms
  _advected_interp_coeffs =
      interpCoeffs(_advected_interp_method, *_current_face_info, true, _volumetric_face_flux);
}
