/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef TENSORMECHANICSPLASTICMEANCAP_H
#define TENSORMECHANICSPLASTICMEANCAP_H

#include "TensorMechanicsPlasticModel.h"
#include "TensorMechanicsHardeningModel.h"

class TensorMechanicsPlasticMeanCap;

template <>
InputParameters validParams<TensorMechanicsPlasticMeanCap>();

/**
 * Class that limits the mean stress
 * Yield function = a*mean_stress - strength
 * mean_stress = (stress_xx + stress_yy + stress_zz)/3
 * a is a real constant, strength is a TensorMechanicsHardening object.
 * Associative
 */
class TensorMechanicsPlasticMeanCap : public TensorMechanicsPlasticModel
{
public:
  TensorMechanicsPlasticMeanCap(const InputParameters & parameters);

  virtual std::string modelName() const override;

protected:
  Real yieldFunction(const RankTwoTensor & stress, Real intnl) const override;

  RankTwoTensor dyieldFunction_dstress(const RankTwoTensor & stress, Real intnl) const override;

  Real dyieldFunction_dintnl(const RankTwoTensor & stress, Real intnl) const override;

  RankTwoTensor flowPotential(const RankTwoTensor & stress, Real intnl) const override;

  RankFourTensor dflowPotential_dstress(const RankTwoTensor & stress, Real intnl) const override;

  RankTwoTensor dflowPotential_dintnl(const RankTwoTensor & stress, Real intnl) const override;

  /// a/3
  Real _a_over_3;

  /// strength
  const TensorMechanicsHardeningModel & _strength;
};

#endif // TENSORMECHANICSPLASTICMEANCAP_H
