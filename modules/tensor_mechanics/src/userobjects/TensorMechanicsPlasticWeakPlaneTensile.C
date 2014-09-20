#include "TensorMechanicsPlasticWeakPlaneTensile.h"

template<>
InputParameters validParams<TensorMechanicsPlasticWeakPlaneTensile>()
{
  InputParameters params = validParams<TensorMechanicsPlasticModel>();
  params.addRequiredRangeCheckedParam<Real>("tensile_strength", "tensile_strength>=0", "Weak plane tensile strength");
  params.addParam<Real>("tensile_strength_residual", "Tenile strength at infinite hardening.  If not given, this defaults to tensile_strength, ie, perfect plasticity");
  params.addRangeCheckedParam<Real>("tensile_strength_rate", 0, "tensile_strength_rate>=0", "Tensile strength = tensile_strenght_residual + (tensile_strength - tensile_strength_residual)*exp(-tensile_rate*plasticstrain).  Set to zero for perfect plasticity");
  params.addClassDescription("Associative weak-plane tensile plasticity with hardening/softening");

  return params;
}

TensorMechanicsPlasticWeakPlaneTensile::TensorMechanicsPlasticWeakPlaneTensile(const std::string & name,
                                                         InputParameters parameters) :
    TensorMechanicsPlasticModel(name, parameters),
    _tension_cutoff(getParam<Real>("tensile_strength")),
    _tension_cutoff_residual(parameters.isParamValid("tensile_strength_residual") ? getParam<Real>("tensile_strength_residual") : _tension_cutoff),
    _tension_cutoff_rate(getParam<Real>("tensile_strength_rate"))
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
  return _tension_cutoff_residual + (_tension_cutoff - _tension_cutoff_residual)*std::exp(-_tension_cutoff_rate*internal_param);
}

Real
TensorMechanicsPlasticWeakPlaneTensile::dtensile_strength(const Real internal_param) const
{
  return -_tension_cutoff_rate*(_tension_cutoff - _tension_cutoff_residual)*std::exp(-_tension_cutoff_rate*internal_param);
}
