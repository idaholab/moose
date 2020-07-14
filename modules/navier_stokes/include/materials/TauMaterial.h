//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#define usingTauMaterialMembers                                                          \
  usingMaterialMembers;                                                                  \
  using TauMaterial::_mass_tau;                                           \
  using TauMaterial::_momentum_tau;                                       \
  using TauMaterial::_energy_tau;                                         \
  using TauMaterial::_h;                                                  \
  using TauMaterial::_N                                                   \

#include "ADMaterial.h"
#include "SinglePhaseFluidProperties.h"
#include "NavierStokesEnums.h"

class TauMaterial;

declareADValidParams(TauMaterial);

/**
 * Material computing $\tau$ for Streamline Upwind Petrov-Galerkin stabilization.
 * $\tau$ is constructed from a variety of intrinsic time scales characterizing the
 * flow - an advection time scale, a diffusive time scale, and a temporal time scale.
 *
 * The advective time scale is computed as either an incompressible form,
 * $\frac{h}{2\|\vec{V}\|}$, or as a compressible form, $\frac{h}{2(\|\vec{V}\|+c}$,
 * where $c$ is the speed of sound.
 *
 * The diffusive time scale is computed as $\frac{h^2}{4\rho_fC_{p,f}/k_f}$.
 *
 * The temporal time scale is computed as $\frac{\Delta t}{2}$.
 *
 * These three time scales are then combined in a variety of different manners to form
 * $\tau$. Each scale may be scaled by an arbitrary constant given as each entry in the
 * `multipliers` parameter for further customization. A single multiplier on the entire
 * $\tau$ expression can also be provided for coarse-grained manipulation.
 */
class TauMaterial : public ADMaterial
{
public:
  TauMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override final;

  /// Compute $\tau$ as a combination of `_mass_tau`, `_momentum_tau`, and `_energy_tau`.
  /// Derived classes may combine these into a single scalar, form a dense matrix, or
  /// form a diagonal matrix.
  virtual void computeTau() = 0;

  /// Compute the incompressible advection limit contribution to $\tau$.
  virtual ADReal computeIncompressibleLimit() const;

  /// Compute the compressible advection limit contribution to $\tau$.
  virtual ADReal computeCompressibleLimit() const;

  /// If the problem is transient, compute the transient limit contribution to $\tau$.
  virtual ADReal computeTransientLimit() const;

  /// Compute the energy diffusive limit contribution to $\tau$.
  virtual ADReal computeEnergyDiffusiveLimit() const;

  /// Compute the momentum diffusive limit contribution to $\tau$.
  virtual ADReal computeMomentumDiffusiveLimit() const;

  /// Combine the advection, diffusive, and temporal limits into a single value
  virtual ADReal computeTauComponent(const std::vector<ADReal> limits) const;

  /**
   * Enumeration representing methods available for combining multiple limiting
   * values into a single value. This can be performed by either selecting
   * the maximum of these three limits (minimum) or by taking the square root of
   * their sum (switching).
   */
  enum TauMethodEnum
  {
    switching,
    minimum
  };

  enum AdvectiveLimitEnum
  {
    incompressible,
    compressible,
    combined
  };

  /// Number of equations in nonlinear system
  const unsigned int _N;

  /// Scaling multiplier to be applied to $\tau$
  const Real & _factor;

  /// Whether the viscous stress term is present in the momentum equation
  const bool & _viscous_stress;

  /// Density
  const ADMaterialProperty<Real> & _rho;

  /// speed
  const ADMaterialProperty<Real> & _speed;

  /// velocity
  const ADMaterialProperty<RealVectorValue> & _velocity;

  /**
   * How to combine the individual transient, advection, and diffusion limits into
   * a single representation for $\tau$.
   */
  const TauMethodEnum _method;

  /// How to select the temporal limit, either on/off
  const settings::OnOffEnum _temporal_limit;

  /// How to select the advective limit, either compressible/incompressible/combined
  const AdvectiveLimitEnum _advective_limit;

  /// How to select the diffusive limit, either on/off
  const settings::OnOffEnum _diffusive_limit;

  /**
   * Multipliers to be applied to each of the individual limits, in the order of
   * temporal, advective, and diffusive.
   */
  const RealVectorValue & _multipliers;

  /// Fluid property object
  const SinglePhaseFluidProperties * _fluid;

  /// Specific volume
  const ADMaterialProperty<Real> * _v;

  /// Specific internal energy
  const ADMaterialProperty<Real> * _e;

  /// Isobaric specific heat
  const ADMaterialProperty<Real> * _cp;

  /// Fluid effective thermal conductivity
  const ADMaterialProperty<Real> * _kappa;

  /// Brinkman viscosity
  const ADMaterialProperty<Real> * _mu_eff;

  /// element size
  Real _h;

  /// Component of $\tau$ to use for the mass equation
  ADReal _mass_tau;

  /// Component of $\tau$ to use for the momentum equations
  ADReal _momentum_tau;

  /// Component of $\tau$ to use for the energy equation
  ADReal _energy_tau;

};
