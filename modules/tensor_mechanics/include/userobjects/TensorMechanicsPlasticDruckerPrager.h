/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef TENSORMECHANICSPLASTICDRUCKERPRAGER_H
#define TENSORMECHANICSPLASTICDRUCKERPRAGER_H

#include "TensorMechanicsPlasticModel.h"
#include "TensorMechanicsHardeningModel.h"


class TensorMechanicsPlasticDruckerPrager;


template<>
InputParameters validParams<TensorMechanicsPlasticDruckerPrager>();

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
  TensorMechanicsPlasticDruckerPrager(const InputParameters & parameters);


  /// Returns the model name (DruckerPrager)
  virtual std::string modelName() const;

 protected:

  /// Hardening model for cohesion
  const TensorMechanicsHardeningModel & _mc_cohesion;

  /// Hardening model for tan(phi)
  const TensorMechanicsHardeningModel & _mc_phi;

  /// Hardening model for tan(psi)
  const TensorMechanicsHardeningModel & _mc_psi;

  /**
   * The yield function
   * @param stress the stress at which to calculate the yield function
   * @param intnl internal parameter
   * @return the yield function
   */
  virtual Real yieldFunction(const RankTwoTensor & stress, Real intnl) const;

  /**
   * The derivative of yield function with respect to stress
   * @param stress the stress at which to calculate the yield function
   * @param intnl internal parameter
   * @return df_dstress(i, j) = dyieldFunction/dstress(i, j)
   */
  virtual RankTwoTensor dyieldFunction_dstress(const RankTwoTensor & stress, Real intnl) const;

  /**
   * The derivative of yield function with respect to the internal parameter
   * @param stress the stress at which to calculate the yield function
   * @param intnl internal parameter
   * @return the derivative
   */
  virtual Real dyieldFunction_dintnl(const RankTwoTensor & stress, Real intnl) const;

  /**
   * The flow potential
   * @param stress the stress at which to calculate the flow potential
   * @param intnl internal parameter
   * @return the flow potential
   */
  virtual RankTwoTensor flowPotential(const RankTwoTensor & stress, Real intnl) const;

  /**
   * The derivative of the flow potential with respect to stress
   * @param stress the stress at which to calculate the flow potential
   * @param intnl internal parameter
   * @return dr_dstress(i, j, k, l) = dr(i, j)/dstress(k, l)
   */
  virtual RankFourTensor dflowPotential_dstress(const RankTwoTensor & stress, Real intnl) const;

  /**
   * The derivative of the flow potential with respect to the internal parameter
   * @param stress the stress at which to calculate the flow potential
   * @param intnl internal parameter
   * @return dr_dintnl(i, j) = dr(i, j)/dintnl
   */
  virtual RankTwoTensor dflowPotential_dintnl(const RankTwoTensor & stress, Real intnl) const;

  const MooseEnum _mc_interpolation_scheme;

  /// True if there is no hardening of cohesion
  const bool _zero_cohesion_hardening;

  /// True if there is no hardening of friction angle
  const bool _zero_phi_hardening;

  /// True if there is no hardening of dilation angle
  const bool _zero_psi_hardening;

  void AandB(Real intnl, Real & aaa, Real & bbb) const;

  void dAandB(Real intnl, Real & daaa, Real & dbbb) const;

  void Bonly(Real intnl, bool friction, Real & bbb) const;

  void dBonly(Real intnl, bool friction, Real & dbbb) const;

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
   * @param friction If true then the friction angle is used to set bbb, otherwise the dilation angle is used
   * @param[out] bbb The Drucker-Prager bbb quantity
   */
  void initializeB(Real intnl, bool friction, Real & bbb) const;
};

#endif // TENSORMECHANICSPLASTICDRUCKERPRAGER_H
