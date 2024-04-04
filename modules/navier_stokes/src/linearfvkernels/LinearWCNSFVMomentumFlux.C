//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearWCNSFVMomentumFlux.h"
#include "NSFVUtils.h"

registerMooseObject("NavierStokesApp", LinearWCNSFVMomentumFlux);

InputParameters
LinearWCNSFVMomentumFlux::validParams()
{
  InputParameters params = LinearFVFluxKernel::validParams();
  params.addClassDescription("Represents the matrix and right hand side contributions of the "
                             "stress and advection terms of the momentum equation.");
  params.addRequiredParam<MooseFunctorName>("advecting_velocity", "Advection velocity");
  params += Moose::FV::interpolationParameters();
  ;
  return params;
}

LinearWCNSFVMomentumFlux::LinearWCNSFVMomentumFlux(const InputParameters & params)
  : LinearFVFluxKernel(params),
    _vel_provider(getUserObject<INSFVRhieChowInterpolatorSegregated>("rhie_chow_user_object")),
    _interp_coeffs(std::make_pair<Real, Real>(0, 0)),
    _face_mass_flux(0.0)

{
  Moose::FV::setInterpolationMethods(*this, _advected_interp_method, _velocity_interp_method);
}

void
LinearWCNSFVMomentumFlux::initialSetup()
{
}

Real
LinearWCNSFVMomentumFlux::computeElemMatrixContribution()
{
  return computeInternalAdvectionElemMatrixContribution() * _current_face_area;
}

Real
LinearWCNSFVMomentumFlux::computeNeighborMatrixContribution()
{
  return computeInternalAdvectionNeighborMatrixContribution() * _current_face_area;
}

Real
LinearWCNSFVMomentumFlux::computeElemRightHandSideContribution()
{
  return 0.0;
}

Real
LinearWCNSFVMomentumFlux::computeNeighborRightHandSideContribution()
{
  return 0.0;
}

Real
LinearWCNSFVMomentumFlux::computeBoundaryMatrixContribution(
    const LinearFVBoundaryCondition & /*bc*/)
{
  return 0.0;
}

Real
LinearWCNSFVMomentumFlux::computeBoundaryRHSContribution(const LinearFVBoundaryCondition & /*bc*/)
{
  return 0.0;
}

Real
LinearWCNSFVMomentumFlux::computeInternalAdvectionElemMatrixContribution()
{
  return _interp_coeffs.first * _face_mass_flux;
}

Real
LinearWCNSFVMomentumFlux::computeInternalAdvectionNeighborMatrixContribution()
{
  return _interp_coeffs.second * _face_mass_flux;
}

void
LinearWCNSFVMomentumFlux::setCurrentFaceInfo(const FaceInfo * face_info)
{
  LinearFVFluxKernel::setCurrentFaceInfo(face_info);

  // First, we fetch the velocity on the face
  const auto & velocity =
      _vel_provider.getVelocity(_velocity_interp_method, *face_info, determineState(), _tid, false);

  // Caching the mass flux on the face which will be reused in the advection term's matrix and right
  // hand side contributions
  _face_mass_flux = raw_value(velocity * _current_face_info->normal());

  // Caching the interpolation coefficients so they will be reused for the matrix and right hand
  // side terms
  _interp_coeffs = interpCoeffs(_advected_interp_method, *_current_face_info, true, velocity);
}
