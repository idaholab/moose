//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CappedDruckerPragerStressUpdate.h"

/**
 * CappedDruckerPragerCosseratStressUpdate performs the return-map
 * algorithm and associated stress updates for plastic
 * models that describe capped Drucker-Prager plasticity in the
 * layered Cosserat setting
 *
 * This plastic model is ideally suited to a material (eg rock)
 * that has different compressive, tensile and shear
 * strengths.  Any of these strengths may be set large to
 * accommodate special situations.  For instance, setting
 * the compressive strength large is appropriate if there
 * is no chance of compressive failure in a model.
 *
 * This plastic model has two internal parameters:
 *  - a "shear" internal parameter which parameterises the
 *    amount of shear plastic strain.  The cohesion, friction
 *    angle, and dilation angle are assumed to be functions
 *    of this internal parameter.
 *  - a "tensile" internal parameter which parameterises the
 *    amount of tensile plastic strain.  The tensile strength
 *    and compressive strength are assumed to be functions of
 *    this internal parameter.  This means, for instance, that
 *    failure in tension can cause the compressive strenght
 *    to soften, as would be expected in rock-mechanics
 *    scenarios.
 *
 * This plasticity model assumes that the flow rule involves an
 * isotropic built from the user-supplied Young and Poisson.
 * That is, the return-map process does NOT flow using the
 * elasticity tensor in the stress-strain law, which is unsymmetric
 * in general (in the Cosserat setting).  Physically this corresponds
 * to the notion that the "host" medium for the Cosserat grains/layers
 * is failing via Drucker-Prager, and not the Cosserat grains/layers
 * themselves.
 */
class CappedDruckerPragerCosseratStressUpdate : public CappedDruckerPragerStressUpdate
{
public:
  static InputParameters validParams();

  CappedDruckerPragerCosseratStressUpdate(const InputParameters & parameters);

  /**
   * Does the model require the elasticity tensor to be isotropic?
   */
  bool requiresIsotropicTensor() override { return false; }

protected:
  /// Shear modulus for the host medium
  const Real _shear;

  /// Isotropic elasticity tensor for the host medium
  RankFourTensor _Ehost;

  virtual void setEppEqq(const RankFourTensor & Eijkl, Real & Epp, Real & Eqq) const override;

  virtual void setStressAfterReturn(const RankTwoTensor & stress_trial,
                                    Real p_ok,
                                    Real q_ok,
                                    Real gaE,
                                    const std::vector<Real> & intnl,
                                    const yieldAndFlow & smoothed_q,
                                    const RankFourTensor & Eijkl,
                                    RankTwoTensor & stress) const override;

  virtual void consistentTangentOperator(const RankTwoTensor & stress_trial,
                                         Real p_trial,
                                         Real q_trial,
                                         const RankTwoTensor & stress,
                                         Real p,
                                         Real q,
                                         Real gaE,
                                         const yieldAndFlow & smoothed_q,
                                         const RankFourTensor & Eijkl,
                                         bool compute_full_tangent_operator,
                                         RankFourTensor & cto) const override;
};
