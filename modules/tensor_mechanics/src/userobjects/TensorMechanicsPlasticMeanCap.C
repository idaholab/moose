/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "TensorMechanicsPlasticMeanCap.h"

template <>
InputParameters
validParams<TensorMechanicsPlasticMeanCap>()
{
  InputParameters params = validParams<TensorMechanicsPlasticModel>();
  params.addParam<Real>("a", 1.0, "Yield function = a*mean_stress - strength");
  params.addRequiredParam<UserObjectName>("strength", "Yield function = a*mean_stress - strength");
  params.addClassDescription("Class that limits the mean stress.  Yield function = a*mean_stress - "
                             "strength.  mean_stress = (stress_xx + stress_yy + stress_zz)/3");

  return params;
}

TensorMechanicsPlasticMeanCap::TensorMechanicsPlasticMeanCap(const InputParameters & parameters)
  : TensorMechanicsPlasticModel(parameters),
    _a_over_3(getParam<Real>("a") / 3.0),
    _strength(getUserObject<TensorMechanicsHardeningModel>("strength"))
{
}

Real
TensorMechanicsPlasticMeanCap::yieldFunction(const RankTwoTensor & stress, Real intnl) const
{
  return _a_over_3 * stress.trace() - _strength.value(intnl);
}

RankTwoTensor
TensorMechanicsPlasticMeanCap::dyieldFunction_dstress(const RankTwoTensor & stress,
                                                      Real /*intnl*/) const
{
  return _a_over_3 * stress.dtrace();
}

Real
TensorMechanicsPlasticMeanCap::dyieldFunction_dintnl(const RankTwoTensor & /*stress*/,
                                                     Real intnl) const
{
  return -_strength.derivative(intnl);
}

RankTwoTensor
TensorMechanicsPlasticMeanCap::flowPotential(const RankTwoTensor & stress, Real /*intnl*/) const
{
  return _a_over_3 * stress.dtrace();
}

RankFourTensor
TensorMechanicsPlasticMeanCap::dflowPotential_dstress(const RankTwoTensor & /*stress*/,
                                                      Real /*intnl*/) const
{
  return RankFourTensor();
}

RankTwoTensor
TensorMechanicsPlasticMeanCap::dflowPotential_dintnl(const RankTwoTensor & /*stress*/,
                                                     Real /*intnl*/) const
{
  return RankTwoTensor();
}

std::string
TensorMechanicsPlasticMeanCap::modelName() const
{
  return "MeanCap";
}
