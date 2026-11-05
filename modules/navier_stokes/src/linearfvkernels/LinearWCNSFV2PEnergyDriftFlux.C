//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearWCNSFV2PEnergyDriftFlux.h"
#include "NS.h"
#include "NavierStokesMethods.h"

registerMooseObject("NavierStokesApp", LinearWCNSFV2PEnergyDriftFlux);

InputParameters
LinearWCNSFV2PEnergyDriftFlux::validParams()
{
  auto params = LinearWCNSFV2PDriftFluxBase::validParams();
  params.addClassDescription(
      "Implements the enthalpy carried by the relative motion of the phases in the two-phase "
      "mixture model, div(beta_d beta_c / rho_m * (cp_d - cp_c) * T * u_slip), on the left hand "
      "side of the mixture energy equation.");
  params.addRequiredParam<MooseFunctorName>("rho_c", "Continuous phase density.");
  params.addRequiredParam<MooseFunctorName>("cp_d", "Dispersed phase specific heat.");
  params.addRequiredParam<MooseFunctorName>("cp_c", "Continuous phase specific heat.");
  // This term advects the temperature, so it needs a boundary value to act on and must only run
  // where the variable carries a boundary condition. 'force_boundary_execution' is deliberately
  // left at its default of false: forcing it hands this kernel a null boundary condition on any
  // boundary the temperature is not constrained on.

  MooseEnum coeff_interp_method("average harmonic", "average");
  params.addParam<MooseEnum>(
      "coeff_interp_method",
      coeff_interp_method,
      "Switch that can select the face interpolation method for the enthalpy flux coefficient.");

  params += Moose::FV::advectedInterpolationParameter();
  return params;
}

LinearWCNSFV2PEnergyDriftFlux::LinearWCNSFV2PEnergyDriftFlux(const InputParameters & params)
  : LinearWCNSFV2PDriftFluxBase(params),
    _rho_c(getFunctor<Real>("rho_c")),
    _cp_d(getFunctor<Real>("cp_d")),
    _cp_c(getFunctor<Real>("cp_c")),
    _coeff_interp_method(
        Moose::FV::selectInterpolationMethod(getParam<MooseEnum>("coeff_interp_method"))),
    _advected_interp_coeffs(std::make_pair<Real, Real>(0, 0)),
    _face_flux(0.0)
{
  Moose::FV::setInterpolationMethod(*this, _advected_interp_method, "advected_interp_method");
}

Real
LinearWCNSFV2PEnergyDriftFlux::computeElemMatrixContribution()
{
  return _advected_interp_coeffs.first * _face_flux * _current_face_area;
}

Real
LinearWCNSFV2PEnergyDriftFlux::computeNeighborMatrixContribution()
{
  return _advected_interp_coeffs.second * _face_flux * _current_face_area;
}

Real
LinearWCNSFV2PEnergyDriftFlux::computeElemRightHandSideContribution()
{
  // The term is linear in the advected temperature, so it is carried entirely by the matrix
  return 0.0;
}

Real
LinearWCNSFV2PEnergyDriftFlux::computeNeighborRightHandSideContribution()
{
  return 0.0;
}

Real
LinearWCNSFV2PEnergyDriftFlux::computeBoundaryMatrixContribution(
    const LinearFVBoundaryCondition & bc)
{
  const auto * const adv_bc = static_cast<const LinearFVAdvectionDiffusionBC *>(&bc);
  mooseAssert(adv_bc, "This should be a valid BC!");

  const auto boundary_value_matrix_contrib = adv_bc->computeBoundaryValueMatrixContribution();

  return boundary_value_matrix_contrib * _boundary_normal_factor * _face_flux * _current_face_area;
}

Real
LinearWCNSFV2PEnergyDriftFlux::computeBoundaryRHSContribution(const LinearFVBoundaryCondition & bc)
{
  const auto * const adv_bc = static_cast<const LinearFVAdvectionDiffusionBC *>(&bc);
  mooseAssert(adv_bc, "This should be a valid BC!");

  const auto boundary_value_rhs_contrib = adv_bc->computeBoundaryValueRHSContribution();

  return -boundary_value_rhs_contrib * _boundary_normal_factor * _face_flux * _current_face_area;
}

void
LinearWCNSFV2PEnergyDriftFlux::setupFaceData(const FaceInfo * face_info)
{
  LinearWCNSFV2PDriftFluxBase::setupFaceData(face_info);

  const auto & normal = _current_face_info->normal();
  const auto state = determineState();
  const auto u_slip_vel_vec = slipVelocity(currentFaceArg(), state);
  const auto face_coefficient = faceCoefficient([this](const auto & arg, const auto & state)
                                                { return enthalpyFluxCoefficient(arg, state); },
                                                state,
                                                _coeff_interp_method);

  _face_flux = NS::slipAllowedOnFace(*_current_face_info, _slip_boundaries)
                   ? face_coefficient * (normal * u_slip_vel_vec)
                   : 0.0;

  // Only internal faces need advected interpolation coefficients; boundary faces are handled
  // through the linear FV boundary conditions
  if (_current_face_type == FaceInfo::VarFaceNeighbors::BOTH)
    _advected_interp_coeffs =
        interpCoeffs(_advected_interp_method, *_current_face_info, true, _face_flux);
}
