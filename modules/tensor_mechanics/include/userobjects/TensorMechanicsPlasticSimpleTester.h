/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef TENSORMECHANICSPLASTICSIMPLETESTER_H
#define TENSORMECHANICSPLASTICSIMPLETESTER_H

#include "TensorMechanicsPlasticModel.h"


class TensorMechanicsPlasticSimpleTester;


template<>
InputParameters validParams<TensorMechanicsPlasticSimpleTester>();

/**
 * Class that can be used for testing multi-surface plasticity models.
 * Yield function = a*stress_yy + b*stress_zz + c*stress_xx + d*(stress_xy + stress_yx)/2 + e*(stress_xz + stress_zx)/2 + f*(stress_yz + stress_zy)/2 - strength
 * No hardening/softening.  Associative.
 */
class TensorMechanicsPlasticSimpleTester : public TensorMechanicsPlasticModel
{
 public:
  TensorMechanicsPlasticSimpleTester(const InputParameters & parameters);

  /// Returns the model name (SimpleTester)
  virtual std::string modelName() const;

 protected:

  /**
   * The yield function
   * @param stress the stress at which to calculate the yield function
   * @param intnl internal parameter
   * @return the yield function
   */
  Real yieldFunction(const RankTwoTensor & stress, Real intnl) const;

  /**
   * The derivative of yield function with respect to stress
   * @param stress the stress at which to calculate the yield function
   * @param intnl internal parameter
   * @return df_dstress(i, j) = dyieldFunction/dstress(i, j)
   */
  RankTwoTensor dyieldFunction_dstress(const RankTwoTensor & stress, Real intnl) const;

  /**
   * The derivative of yield function with respect to the internal parameter
   * @param stress the stress at which to calculate the yield function
   * @param intnl internal parameter
   * @return the derivative
   */
  Real dyieldFunction_dintnl(const RankTwoTensor & stress, Real intnl) const;

  /**
   * The flow potential
   * @param stress the stress at which to calculate the flow potential
   * @param intnl internal parameter
   * @return the flow potential
   */
  RankTwoTensor flowPotential(const RankTwoTensor & stress, Real intnl) const;

  /**
   * The derivative of the flow potential with respect to stress
   * @param stress the stress at which to calculate the flow potential
   * @param intnl internal parameter
   * @return dr_dstress(i, j, k, l) = dr(i, j)/dstress(k, l)
   */
  RankFourTensor dflowPotential_dstress(const RankTwoTensor & stress, Real intnl) const;

  /**
   * The derivative of the flow potential with respect to the internal parameter
   * @param stress the stress at which to calculate the flow potential
   * @param intnl internal parameter
   * @return dr_dintnl(i, j) = dr(i, j)/dintnl
   */
  RankTwoTensor dflowPotential_dintnl(const RankTwoTensor & stress, Real intnl) const;

  /// a
  Real _a;

  /// b
  Real _b;

  /// c
  Real _c;

  /// d
  Real _d;

  /// e
  Real _e;

  /// f
  Real _f;

  /// strength
  Real _strength;
};

#endif // TENSORMECHANICSPLASTICSIMPLETESTER_H
