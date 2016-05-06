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

  /// Calculates aaa and bbb as a function of the internal parameter intnl
  void AandB(Real intnl, Real & aaa, Real & bbb) const;

  /// Calculates d(aaa)/d(intnl) and d(bbb)/d(intnl) as a function of the internal parameter intnl
  void dAandB(Real intnl, Real & daaa, Real & dbbb) const;

  /**
   * bbb (friction) and bbb_flow (dilation) are computed using the same function,
   * Bonly, and this parameter tells that function whether to compute bbb or bbb_flow
   */
  enum fric_or_dil { friction = 0, dilation = 1 };

  /**
   * Calculate bbb or bbb_flow
   * @param intnl the internal parameter
   * @param fd if fd==friction then bbb is calculated.  if fd==dilation then bbb_flow is calculated
   * @param bbb either bbb or bbb_flow, depending on fd
   */
  void Bonly(Real intnl, int fd, Real & bbb) const;

  /**
   * Calculate d(bbb)/d(intnl) or d(bbb_flow)/d(intnl)
   * @param intnl the internal parameter
   * @param fd if fd==friction then bbb is calculated.  if fd==dilation then bbb_flow is calculated
   * @param bbb either bbb or bbb_flow, depending on fd
   */
  void dBonly(Real intnl, int fd, Real & dbbb) const;

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
   * @param fd If fd == frction then the friction angle is used to set bbb, otherwise the dilation angle is used
   * @param[out] bbb The Drucker-Prager bbb quantity
   */
  void initializeB(Real intnl, int fd, Real & bbb) const;
};

#endif // TENSORMECHANICSPLASTICDRUCKERPRAGER_H
