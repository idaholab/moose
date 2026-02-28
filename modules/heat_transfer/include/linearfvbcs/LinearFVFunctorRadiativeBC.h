//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LinearFVAdvectionDiffusionFunctorRobinBCBase.h"

/**
 * Boundary condition for radiative heat flux in a linear finite volume system.
 * The radiative flux q = sigma * emissivity * (T^4 - Tinfinity^4) is Newton-linearized
 * around the extrapolated boundary face temperature T_b_old at each assembly step:
 *   k * dT/dn + [4*sigma*eps*T_b_old^3]*T_b = [sigma*eps*(3*T_b_old^4 + Tinf^4)]
 * This is a Robin BC with alpha=k, beta=4*sigma*eps*T_b_old^3, gamma=sigma*eps*(3*T_b_old^4+Tinf^4),
 * yielding second-order spatial accuracy. Outer Picard convergence is driven by transient
 * stepping or a coupled nonlinear solve.
 */
class LinearFVFunctorRadiativeBC : public LinearFVAdvectionDiffusionFunctorRobinBCBase
{
public:
  static InputParameters validParams();

  LinearFVFunctorRadiativeBC(const InputParameters & parameters);

protected:
  virtual Real getAlpha(Moose::FaceArg face, Moose::StateArg state) const override;
  virtual Real getBeta(Moose::FaceArg face, Moose::StateArg state) const override;
  virtual Real getGamma(Moose::FaceArg face, Moose::StateArg state) const override;

private:
  /// Extrapolates the boundary face temperature from the previous Picard iteration.
  Real extrapolateFaceTemperature(Moose::StateArg state) const;

  /// Emissivity functor (epsilon)
  const Moose::Functor<Real> & _emissivity;

  /// Far-field temperature functor (T_infinity)
  const Moose::Functor<Real> & _tinf;

  /// Stefan-Boltzmann constant (sigma)
  const Real _sigma;

  /// Thermal conductivity functor; must match the diffusion_coeff in LinearFVDiffusion
  const Moose::Functor<Real> & _diffusion_coeff;
};
