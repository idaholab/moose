//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TensorMechanicsPlasticMeanCap.h"
#include "RankFourTensor.h"

registerMooseObject("TensorMechanicsApp", TensorMechanicsPlasticMeanCap);

InputParameters
TensorMechanicsPlasticMeanCap::validParams()
{
  InputParameters params = TensorMechanicsPlasticModel::validParams();
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
