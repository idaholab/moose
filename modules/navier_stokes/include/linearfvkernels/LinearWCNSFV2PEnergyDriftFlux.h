//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LinearWCNSFV2PDriftFluxBase.h"
#include "LinearFVAdvectionDiffusionBC.h"
#include "MathFVUtils.h"

/**
 * Enthalpy carried by the relative motion of the phases, the energy counterpart of the diffusion
 * (drift) stress in the mixture momentum equation.
 *
 * For one dispersed phase, with both phases at the mixture temperature, this kernel assembles
 * \f$ \nabla \cdot \left[ \frac{\beta_d \beta_c}{\rho_m} (c_{p,d} - c_{p,c}) T u_s \right] \f$
 * on the left hand side, beyond what LinearFVEnergyAdvection assembles. It vanishes when the two
 * specific heats are equal. The derivation is in the documentation page.
 */
class LinearWCNSFV2PEnergyDriftFlux : public LinearWCNSFV2PDriftFluxBase
{
public:
  static InputParameters validParams();
  LinearWCNSFV2PEnergyDriftFlux(const InputParameters & params);

  virtual Real computeElemMatrixContribution() override;

  virtual Real computeNeighborMatrixContribution() override;

  virtual Real computeElemRightHandSideContribution() override;

  virtual Real computeNeighborRightHandSideContribution() override;

  virtual Real computeBoundaryMatrixContribution(const LinearFVBoundaryCondition & bc) override;

  virtual Real computeBoundaryRHSContribution(const LinearFVBoundaryCondition & bc) override;

  virtual void setupFaceData(const FaceInfo * face_info) override;

protected:
  /// Continuous phase density
  const Moose::Functor<Real> & _rho_c;

  /// Dispersed phase specific heat
  const Moose::Functor<Real> & _cp_d;

  /// Continuous phase specific heat
  const Moose::Functor<Real> & _cp_c;

  /// The face interpolation method for the coefficient
  const Moose::FV::InterpMethod _coeff_interp_method;

  /**
   * The enthalpy flux coefficient on a face or element,
   * beta_d beta_c / rho_m * (cp_d - cp_c), with the phase fraction clamped into [0, 1].
   */
  template <typename SpaceArg>
  Real enthalpyFluxCoefficient(const SpaceArg & arg, const Moose::StateArg & state) const
  {
    return NS::diffusionStressCoefficient(
               _f_d(arg, state), _rho_d(arg, state), _rho_c(arg, state)) *
           (_cp_d(arg, state) - _cp_c(arg, state));
  }

private:
  /// Container for the current advected interpolation coefficients on the face, so that they are
  /// only computed once per face
  std::pair<Real, Real> _advected_interp_coeffs;

  /// The enthalpy flux on the face, cached by setupFaceData
  Real _face_flux;

  /// The interpolation method to use for the advected temperature
  Moose::FV::InterpMethod _advected_interp_method;
};
