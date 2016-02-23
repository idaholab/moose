/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef TENSORMECHANICSPLASTICWEAKPLANETENSILEN_H
#define TENSORMECHANICSPLASTICWEAKPLANETENSILEN_H

#include "TensorMechanicsPlasticWeakPlaneTensile.h"


class TensorMechanicsPlasticWeakPlaneTensileN;


template<>
InputParameters validParams<TensorMechanicsPlasticWeakPlaneTensileN>();

/**
 * Rate-independent associative weak-plane tensile failure
 * with hardening/softening, and normal direction specified
 */
class TensorMechanicsPlasticWeakPlaneTensileN : public TensorMechanicsPlasticWeakPlaneTensile
{
 public:
  TensorMechanicsPlasticWeakPlaneTensileN(const InputParameters & parameters);

  /// Returns the model name (WeakPlaneTensileN)
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


  /// Unit normal inputted by user
  RealVectorValue _input_n;

  /// Flow direction, which is constant in this case
  RankTwoTensor _df_dsig;

  /// This rotation matrix rotates _input_n to (0, 0, 1)
  RealTensorValue _rot;
};

#endif // TENSORMECHANICSPLASTICWEAKPLANETENSILEN_H
