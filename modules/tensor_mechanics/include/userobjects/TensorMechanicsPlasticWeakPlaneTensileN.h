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

template <>
InputParameters validParams<TensorMechanicsPlasticWeakPlaneTensileN>();

/**
 * Rate-independent associative weak-plane tensile failure
 * with hardening/softening, and normal direction specified
 */
class TensorMechanicsPlasticWeakPlaneTensileN : public TensorMechanicsPlasticWeakPlaneTensile
{
public:
  TensorMechanicsPlasticWeakPlaneTensileN(const InputParameters & parameters);

  virtual std::string modelName() const override;

protected:
  Real yieldFunction(const RankTwoTensor & stress, Real intnl) const override;

  RankTwoTensor dyieldFunction_dstress(const RankTwoTensor & stress, Real intnl) const override;

  Real dyieldFunction_dintnl(const RankTwoTensor & stress, Real intnl) const override;

  RankTwoTensor flowPotential(const RankTwoTensor & stress, Real intnl) const override;

  RankFourTensor dflowPotential_dstress(const RankTwoTensor & stress, Real intnl) const override;

  RankTwoTensor dflowPotential_dintnl(const RankTwoTensor & stress, Real intnl) const override;

  /// Unit normal inputted by user
  RealVectorValue _input_n;

  /// Flow direction, which is constant in this case
  RankTwoTensor _df_dsig;

  /// This rotation matrix rotates _input_n to (0, 0, 1)
  RealTensorValue _rot;
};

#endif // TENSORMECHANICSPLASTICWEAKPLANETENSILEN_H
