#include "TensorMechanicsPlasticWeakPlaneTensile.h"

template<>
InputParameters validParams<TensorMechanicsPlasticWeakPlaneTensile>()
{
  InputParameters params = validParams<TensorMechanicsPlasticModel>();
  params.addClassDescription("Associative weak-plane tensile plasticity with tensile strength = 1");

  return params;
}

TensorMechanicsPlasticWeakPlaneTensile::TensorMechanicsPlasticWeakPlaneTensile(const std::string & name,
                                                         InputParameters parameters) :
    TensorMechanicsPlasticModel(name, parameters)
{
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
  return 1.0;
}

Real
TensorMechanicsPlasticWeakPlaneTensile::dtensile_strength(const Real internal_param) const
{
  return 0.0;
}
