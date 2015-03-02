/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef TENSORMECHANICSPLASTICJ2_H
#define TENSORMECHANICSPLASTICJ2_H

#include "TensorMechanicsPlasticModel.h"
#include "TensorMechanicsHardeningModel.h"


class TensorMechanicsPlasticJ2;


template<>
InputParameters validParams<TensorMechanicsPlasticJ2>();

/**
 * J2 plasticity, associative, with hardning.
 * Yield_function = sqrt(3*J2) - yield_strength
 */
class TensorMechanicsPlasticJ2 : public TensorMechanicsPlasticModel
{
 public:
  TensorMechanicsPlasticJ2(const std::string & name, InputParameters parameters);

  /// returns the model name (J2)
  virtual std::string modelName() const;

 protected:

  /**
   * The yield function
   * @param stress the stress at which to calculate the yield function
   * @param intnl internal parameter
   * @return the yield function
   */
  Real yieldFunction(const RankTwoTensor & stress, const Real & intnl) const;

  /**
   * The derivative of yield function with respect to stress
   * @param stress the stress at which to calculate the yield function
   * @param intnl internal parameter
   * @return df_dstress(i, j) = dyieldFunction/dstress(i, j)
   */
  RankTwoTensor dyieldFunction_dstress(const RankTwoTensor & stress, const Real & intnl) const;

  /**
   * The derivative of yield function with respect to the internal parameter
   * @param stress the stress at which to calculate the yield function
   * @param intnl internal parameter
   * @return the derivative
   */
  Real dyieldFunction_dintnl(const RankTwoTensor & stress, const Real & intnl) const;

  /**
   * The flow potential
   * @param stress the stress at which to calculate the flow potential
   * @param intnl internal parameter
   * @return the flow potential
   */
  RankTwoTensor flowPotential(const RankTwoTensor & stress, const Real & intnl) const;

  /**
   * The derivative of the flow potential with respect to stress
   * @param stress the stress at which to calculate the flow potential
   * @param intnl internal parameter
   * @return dr_dstress(i, j, k, l) = dr(i, j)/dstress(k, l)
   */
  RankFourTensor dflowPotential_dstress(const RankTwoTensor & stress, const Real & intnl) const;

  /**
   * The derivative of the flow potential with respect to the internal parameter
   * @param stress the stress at which to calculate the flow potential
   * @param intnl internal parameter
   * @return dr_dintnl(i, j) = dr(i, j)/dintnl
   */
  RankTwoTensor dflowPotential_dintnl(const RankTwoTensor & stress, const Real & intnl) const;

  /**
   * YieldStrength.  The yield function is sqrt(3*J2) - yieldStrength.
   * In this class yieldStrength = 1, but this
   * may be over-ridden by derived classes with nontrivial hardning
   */
  virtual Real yieldStrength(const Real & intnl) const;

  /// d(yieldStrength)/d(intnl)
  virtual Real dyieldStrength(const Real & intnl) const;

  const TensorMechanicsHardeningModel & _strength;


};

#endif // TENSORMECHANICSPLASTICJ2_H
