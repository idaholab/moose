/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef CAPPEDMOHRCOULOMBCOSSERATSTRESSUPDATE_H
#define CAPPEDMOHRCOULOMBCOSSERATSTRESSUPDATE_H

#include "CappedMohrCoulombStressUpdate.h"

class CappedMohrCoulombCosseratStressUpdate;

template <>
InputParameters validParams<CappedMohrCoulombCosseratStressUpdate>();

/**
 * CappedMohrCoulombCosseratStressUpdate implements rate-independent nonassociative
 * Mohr-CoulombC plus tensile plus compressive plasticity with hardening/softening
 * in the Cosserat setting.  The Mohr-Coulomb plasticity is based on the symmetric
 * part of the stress tensor only, and uses an isotropic elasticity tensor.
 */
class CappedMohrCoulombCosseratStressUpdate : public CappedMohrCoulombStressUpdate
{
public:
  CappedMohrCoulombCosseratStressUpdate(const InputParameters & parameters);

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

#endif // CAPPEDMOHRCOULOMBCOSSERATSTRESSUPDATE_H
