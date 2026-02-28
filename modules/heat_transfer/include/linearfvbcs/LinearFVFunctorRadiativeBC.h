//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LinearFVAdvectionDiffusionBC.h"

/**
 * Boundary condition for radiative heat flux in a linear finite volume system.
 * The radiative flux q = sigma * emissivity * (T^4 - Tinfinity^4) is Newton-linearized
 * around the current solution (T_old) at each assembly:
 *   q ≈ [4*sigma*eps*T_old^3]*T - [sigma*eps*(3*T_old^4 + Tinf^4)]
 * yielding a matrix contribution and an explicit RHS contribution.
 * Outer Picard convergence is driven by transient stepping or a coupled nonlinear solve.
 */
class LinearFVFunctorRadiativeBC : public LinearFVAdvectionDiffusionBC
{
public:
  static InputParameters validParams();

  LinearFVFunctorRadiativeBC(const InputParameters & parameters);

  virtual Real computeBoundaryValue() const override;
  virtual Real computeBoundaryNormalGradient() const override;
  virtual Real computeBoundaryValueMatrixContribution() const override;
  virtual Real computeBoundaryValueRHSContribution() const override;
  virtual Real computeBoundaryGradientMatrixContribution() const override;
  virtual Real computeBoundaryGradientRHSContribution() const override;

  /// The contributions already represent the total heat flux (no additional k multiplication).
  virtual bool includesMaterialPropertyMultiplier() const override { return true; }

protected:
  /// Emissivity functor (epsilon)
  const Moose::Functor<Real> & _emissivity;

  /// Far-field temperature functor (T_infinity)
  const Moose::Functor<Real> & _tinf;

  /// Stefan-Boltzmann constant (sigma)
  const Real _sigma;

  /// Thermal conductivity functor, used for boundary gradient/value extrapolation
  const Moose::Functor<Real> & _diffusion_coeff;
};
