#include "TensorMechanicsPlasticWeakPlaneTensile.h"

template<>
InputParameters validParams<TensorMechanicsPlasticWeakPlaneTensile>()
{
  InputParameters params = validParams<TensorMechanicsPlasticModel>();
  params.addRequiredParam<UserObjectName>("tensile_strength", "A TensorMechanicsHardening UserObject that defines hardening of the weak-plane tensile strength");
  params.addClassDescription("Associative weak-plane tensile plasticity with hardening/softening");

  return params;
}

TensorMechanicsPlasticWeakPlaneTensile::TensorMechanicsPlasticWeakPlaneTensile(const std::string & name,
                                                         InputParameters parameters) :
    TensorMechanicsPlasticModel(name, parameters),
    _strength(getUserObject<TensorMechanicsHardeningModel>("tensile_strength"))
{
  // cannot check the following for all values of strength, but this is a start
  if (_strength.value(0) < 0)
    mooseError("Weak plane tensile strength must not be negative");
}


Real
TensorMechanicsPlasticWeakPlaneTensile::yieldFunction(const RankTwoTensor & stress, const Real & intnl) const
{
  return stress(2,2) - tensile_strength(intnl);
}

RankTwoTensor
TensorMechanicsPlasticWeakPlaneTensile::dyieldFunction_dstress(const RankTwoTensor & /*stress*/, const Real & /*intnl*/) const
{
  RankTwoTensor df_dsig;
  df_dsig(2, 2) = 1.0;
  return df_dsig;
}


Real
TensorMechanicsPlasticWeakPlaneTensile::dyieldFunction_dintnl(const RankTwoTensor & /*stress*/, const Real & intnl) const
{
  return -dtensile_strength(intnl);
}

RankTwoTensor
TensorMechanicsPlasticWeakPlaneTensile::flowPotential(const RankTwoTensor & /*stress*/, const Real & /*intnl*/) const
{
  RankTwoTensor df_dsig;
  df_dsig(2, 2) = 1.0;
  return df_dsig;
}

RankFourTensor
TensorMechanicsPlasticWeakPlaneTensile::dflowPotential_dstress(const RankTwoTensor & /*stress*/, const Real & /*intnl*/) const
{
  return RankFourTensor();
}

RankTwoTensor
TensorMechanicsPlasticWeakPlaneTensile::dflowPotential_dintnl(const RankTwoTensor & /*stress*/, const Real & /*intnl*/) const
{
  return RankTwoTensor();
}

Real
TensorMechanicsPlasticWeakPlaneTensile::tensile_strength(const Real internal_param) const
{
  return _strength.value(internal_param);
}

Real
TensorMechanicsPlasticWeakPlaneTensile::dtensile_strength(const Real internal_param) const
{
  return _strength.derivative(internal_param);
}

std::string
TensorMechanicsPlasticWeakPlaneTensile::modelName() const
{
  return "WeakPlaneTensile";
}
