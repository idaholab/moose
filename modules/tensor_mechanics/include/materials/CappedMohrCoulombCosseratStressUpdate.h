//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CappedMohrCoulombStressUpdate.h"

/**
 * CappedMohrCoulombCosseratStressUpdate implements rate-independent nonassociative
 * Mohr-Coulomb plus tensile plus compressive plasticity with hardening/softening
 * in the Cosserat setting.  The Mohr-Coulomb plasticity considers the symmetric
 * part of the stress tensor only, and uses an isotropic elasticity tensor that
 * is input by the user (the anti-symmetric parts of the stress tensor and the
 * moment-stress tensor are not included in this plastic model, and any non-isometric
 * parts of the elasticity tensor are ignored in the flow rule).
 */
class CappedMohrCoulombCosseratStressUpdate : public CappedMohrCoulombStressUpdate
{
public:
  static InputParameters validParams();

  CappedMohrCoulombCosseratStressUpdate(const InputParameters & parameters);

  /**
   * The full elasticity tensor may be anisotropic, and usually is in the case
   * of layered Cosserat.  However, this class only uses the isotropic parts of
   * it (corresponding to the "host" material) that are encoded in _host_young
   * and _host_poisson
   */
  bool requiresIsotropicTensor() override { return false; }

protected:
  /// Young's modulus of the host material
  const Real _host_young;

  /// Poisson's of the host material
  const Real _host_poisson;

  /// E0011 = Lame lambda modulus of the host material
  const Real _host_E0011;

  /// E0000 = Lame lambda + 2 * shear modulus of the host material
  const Real _host_E0000;

  virtual void setStressAfterReturnV(const RankTwoTensor & stress_trial,
                                     const std::vector<Real> & stress_params,
                                     Real gaE,
                                     const std::vector<Real> & intnl,
                                     const yieldAndFlow & smoothed_q,
                                     const RankFourTensor & Eijkl,
                                     RankTwoTensor & stress) const override;

  virtual void preReturnMapV(const std::vector<Real> & trial_stress_params,
                             const RankTwoTensor & stress_trial,
                             const std::vector<Real> & intnl_old,
                             const std::vector<Real> & yf,
                             const RankFourTensor & Eijkl) override;

  void setEffectiveElasticity(const RankFourTensor & Eijkl) override;

  virtual void consistentTangentOperatorV(const RankTwoTensor & stress_trial,
                                          const std::vector<Real> & trial_stress_params,
                                          const RankTwoTensor & stress,
                                          const std::vector<Real> & stress_params,
                                          Real gaE,
                                          const yieldAndFlow & smoothed_q,
                                          const RankFourTensor & Eijkl,
                                          bool compute_full_tangent_operator,
                                          const std::vector<std::vector<Real>> & dvar_dtrial,
                                          RankFourTensor & cto) override;
};
