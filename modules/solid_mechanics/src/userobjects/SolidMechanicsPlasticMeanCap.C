//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SolidMechanicsPlasticMeanCap.h"
#include "RankFourTensor.h"

registerMooseObject("SolidMechanicsApp", SolidMechanicsPlasticMeanCap);
registerMooseObjectRenamed("SolidMechanicsApp",
                           TensorMechanicsPlasticMeanCap,
                           "01/01/2025 00:00",
                           SolidMechanicsPlasticMeanCap);

InputParameters
SolidMechanicsPlasticMeanCap::validParams()
{
  InputParameters params = SolidMechanicsPlasticModel::validParams();
  params.addParam<Real>("a", 1.0, "Yield function = a*mean_stress - strength");
  params.addRequiredParam<UserObjectName>("strength", "Yield function = a*mean_stress - strength");
  params.addClassDescription("Class that limits the mean stress.  Yield function = a*mean_stress - "
                             "strength.  mean_stress = (stress_xx + stress_yy + stress_zz)/3");

  return params;
}

SolidMechanicsPlasticMeanCap::SolidMechanicsPlasticMeanCap(const InputParameters & parameters)
  : SolidMechanicsPlasticModel(parameters),
    _a_over_3(getParam<Real>("a") / 3.0),
    _strength(getUserObject<SolidMechanicsHardeningModel>("strength"))
{
}

Real
SolidMechanicsPlasticMeanCap::yieldFunction(const RankTwoTensor & stress, Real intnl) const
{
  return _a_over_3 * stress.trace() - _strength.value(intnl);
}

RankTwoTensor
SolidMechanicsPlasticMeanCap::dyieldFunction_dstress(const RankTwoTensor & stress,
                                                     Real /*intnl*/) const
{
  return _a_over_3 * stress.dtrace();
}

Real
SolidMechanicsPlasticMeanCap::dyieldFunction_dintnl(const RankTwoTensor & /*stress*/,
                                                    Real intnl) const
{
  return -_strength.derivative(intnl);
}

RankTwoTensor
SolidMechanicsPlasticMeanCap::flowPotential(const RankTwoTensor & stress, Real /*intnl*/) const
{
  return _a_over_3 * stress.dtrace();
}

RankFourTensor
SolidMechanicsPlasticMeanCap::dflowPotential_dstress(const RankTwoTensor & /*stress*/,
                                                     Real /*intnl*/) const
{
  return RankFourTensor();
}

RankTwoTensor
SolidMechanicsPlasticMeanCap::dflowPotential_dintnl(const RankTwoTensor & /*stress*/,
                                                    Real /*intnl*/) const
{
  return RankTwoTensor();
}

std::string
SolidMechanicsPlasticMeanCap::modelName() const
{
  return "MeanCap";
}
