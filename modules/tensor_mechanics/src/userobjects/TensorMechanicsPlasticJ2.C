/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "TensorMechanicsPlasticJ2.h"

template<>
InputParameters validParams<TensorMechanicsPlasticJ2>()
{
  InputParameters params = validParams<TensorMechanicsPlasticModel>();
  params.addRequiredParam<UserObjectName>("yield_strength", "A TensorMechanicsHardening UserObject that defines hardening of the yield strength");
  params.addClassDescription("J2 plasticity, associative, with hardening");

  return params;
}

TensorMechanicsPlasticJ2::TensorMechanicsPlasticJ2(const std::string & name,
                                                         InputParameters parameters) :
    TensorMechanicsPlasticModel(name, parameters),
    _strength(getUserObject<TensorMechanicsHardeningModel>("yield_strength"))
{
}


Real
TensorMechanicsPlasticJ2::yieldFunction(const RankTwoTensor & stress, const Real & intnl) const
{
  return std::pow(3*stress.secondInvariant(), 0.5) - yieldStrength(intnl);
}

RankTwoTensor
TensorMechanicsPlasticJ2::dyieldFunction_dstress(const RankTwoTensor & stress, const Real & /*intnl*/) const
{
  Real sII = stress.secondInvariant();
  if (sII == 0.0)
    return RankTwoTensor();
  else
    return 0.5*std::pow(3/sII, 0.5)*stress.dsecondInvariant();
}


Real
TensorMechanicsPlasticJ2::dyieldFunction_dintnl(const RankTwoTensor & /*stress*/, const Real & intnl) const
{
  return -dyieldStrength(intnl);
}

RankTwoTensor
TensorMechanicsPlasticJ2::flowPotential(const RankTwoTensor & stress, const Real & intnl) const
{
  return dyieldFunction_dstress(stress, intnl);
}

RankFourTensor
TensorMechanicsPlasticJ2::dflowPotential_dstress(const RankTwoTensor & stress, const Real & /*intnl*/) const
{
  Real sII = stress.secondInvariant();
  if (sII == 0)
    return RankFourTensor();

  RankFourTensor dfp = 0.5*std::pow(3/sII, 0.5)*stress.d2secondInvariant();
  Real pre = -0.25*std::pow(3, 0.5)*std::pow(sII, -1.5);
  RankTwoTensor dII = stress.dsecondInvariant();
  for (unsigned i = 0 ; i < 3 ; ++i)
    for (unsigned j = 0 ; j < 3 ; ++j)
      for (unsigned k = 0 ; k < 3 ; ++k)
        for (unsigned l = 0 ; l < 3 ; ++l)
          dfp(i, j, k, l) += pre*dII(i, j)*dII(k, l);
  return dfp;
}

RankTwoTensor
TensorMechanicsPlasticJ2::dflowPotential_dintnl(const RankTwoTensor & /*stress*/, const Real & /*intnl*/) const
{
  return RankTwoTensor();
}

Real
TensorMechanicsPlasticJ2::yieldStrength(const Real & intnl) const
{
  return _strength.value(intnl);
}

Real
TensorMechanicsPlasticJ2::dyieldStrength(const Real & intnl) const
{
  return _strength.derivative(intnl);
}

std::string
TensorMechanicsPlasticJ2::modelName() const
{
  return "J2";
}
