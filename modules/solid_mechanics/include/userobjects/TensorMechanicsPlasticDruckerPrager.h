//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TensorMechanicsPlasticModel.h"
#include "TensorMechanicsHardeningModel.h"

/**
 * Rate-independent non-associative Drucker Prager
 * with hardening/softening.  The cone's tip is not smoothed.
 * f = sqrt(J2) + bbb*Tr(stress) - aaa
 * with aaa = "cohesion" (a non-negative number)
 * and bbb = tan(friction angle) (a positive number)
 */
class TensorMechanicsPlasticDruckerPrager : public TensorMechanicsPlasticModel
{
public:
  static InputParameters validParams();

  TensorMechanicsPlasticDruckerPrager(const InputParameters & parameters);

  virtual std::string modelName() const override;

  /// Calculates aaa and bbb as a function of the internal parameter intnl
  void bothAB(Real intnl, Real & aaa, Real & bbb) const;

  /// Calculates d(aaa)/d(intnl) and d(bbb)/d(intnl) as a function of the internal parameter intnl
  void dbothAB(Real intnl, Real & daaa, Real & dbbb) const;

  /**
   * bbb (friction) and bbb_flow (dilation) are computed using the same function,
   * onlyB, and this parameter tells that function whether to compute bbb or bbb_flow
   */
  enum FrictionDilation
  {
    friction = 0,
    dilation = 1
  };

  /**
   * Calculate bbb or bbb_flow
   * @param intnl the internal parameter
   * @param fd if fd==friction then bbb is calculated.  if fd==dilation then bbb_flow is calculated
   * @param bbb either bbb or bbb_flow, depending on fd
   */
  void onlyB(Real intnl, int fd, Real & bbb) const;

  /**
   * Calculate d(bbb)/d(intnl) or d(bbb_flow)/d(intnl)
   * @param intnl the internal parameter
   * @param fd if fd==friction then bbb is calculated.  if fd==dilation then bbb_flow is calculated
   * @param bbb either bbb or bbb_flow, depending on fd
   */
  void donlyB(Real intnl, int fd, Real & dbbb) const;

protected:
  /// Hardening model for cohesion
  const TensorMechanicsHardeningModel & _mc_cohesion;

  /// Hardening model for tan(phi)
  const TensorMechanicsHardeningModel & _mc_phi;

  /// Hardening model for tan(psi)
  const TensorMechanicsHardeningModel & _mc_psi;

  virtual Real yieldFunction(const RankTwoTensor & stress, Real intnl) const override;

  virtual RankTwoTensor dyieldFunction_dstress(const RankTwoTensor & stress,
                                               Real intnl) const override;

  virtual Real dyieldFunction_dintnl(const RankTwoTensor & stress, Real intnl) const override;

  virtual RankTwoTensor flowPotential(const RankTwoTensor & stress, Real intnl) const override;

  virtual RankFourTensor dflowPotential_dstress(const RankTwoTensor & stress,
                                                Real intnl) const override;

  virtual RankTwoTensor dflowPotential_dintnl(const RankTwoTensor & stress,
                                              Real intnl) const override;

  /**
   * The parameters aaa and bbb are chosen to closely match the Mohr-Coulomb
   * yield surface.  Various matching schemes may be used and this parameter
   * holds the user's choice.
   */
  const MooseEnum _mc_interpolation_scheme;

  /// True if there is no hardening of cohesion
  const bool _zero_cohesion_hardening;

  /// True if there is no hardening of friction angle
  const bool _zero_phi_hardening;

  /// True if there is no hardening of dilation angle
  const bool _zero_psi_hardening;

  /// Function that's used in dyieldFunction_dstress and flowPotential
  virtual RankTwoTensor df_dsig(const RankTwoTensor & stress, Real bbb) const;

private:
  Real _aaa;
  Real _bbb;
  Real _bbb_flow;

  /**
   * Returns the Drucker-Prager parameters
   * A nice reference on the different relationships between
   * Drucker-Prager and Mohr-Coulomb is
   * H Jiang and X Yongli, A note on the Mohr-Coulomb and Drucker-Prager strength criteria,
   * Mechanics Research Communications 38 (2011) 309-314.
   * This function uses Table1 in that reference, with the following translations
   * aaa = k
   * bbb = -alpha/3
   * @param intnl The internal parameter
   * @param[out] aaa The Drucker-Prager aaa quantity
   * @param[out] bbb The Drucker-Prager bbb quantity
   */
  void initializeAandB(Real intnl, Real & aaa, Real & bbb) const;

  /**
   * Returns the Drucker-Prager parameters
   * A nice reference on the different relationships between
   * Drucker-Prager and Mohr-Coulomb is
   * H Jiang and X Yongli, A note on the Mohr-Coulomb and Drucker-Prager strength criteria,
   * Mechanics Research Communications 38 (2011) 309-314.
   * This function uses Table1 in that reference, with the following translations
   * aaa = k
   * bbb = -alpha/3
   * @param intnl The internal parameter
   * @param fd If fd == frction then the friction angle is used to set bbb, otherwise the dilation
   * angle is used
   * @param[out] bbb The Drucker-Prager bbb quantity
   */
  void initializeB(Real intnl, int fd, Real & bbb) const;
};
