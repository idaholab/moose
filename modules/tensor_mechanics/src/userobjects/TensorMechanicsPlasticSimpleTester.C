#include "TensorMechanicsPlasticSimpleTester.h"

template<>
InputParameters validParams<TensorMechanicsPlasticSimpleTester>()
{
  InputParameters params = validParams<TensorMechanicsPlasticModel>();
  params.addRequiredParam<Real>("a", "Yield function = a*stress_yy + b*stress_zz - strength");
  params.addRequiredParam<Real>("b", "Yield function = a*stress_yy + b*stress_zz - strength");
  params.addRequiredParam<Real>("strength", "Yield function = a*stress_yy + b*stress_zz - strength");
  params.addClassDescription("Class that can be used for testing multi-surface plasticity models.  Yield function = a*stress_yy + b*stress_zz - strength");

  return params;
}

TensorMechanicsPlasticSimpleTester::TensorMechanicsPlasticSimpleTester(const std::string & name,
                                                         InputParameters parameters) :
    TensorMechanicsPlasticModel(name, parameters),
    _a(getParam<Real>("a")),
    _b(getParam<Real>("b")),
    _strength(getParam<Real>("strength"))
{
}


Real
TensorMechanicsPlasticSimpleTester::yieldFunction(const RankTwoTensor & stress, const Real & /*intnl*/) const
{
  return _a*stress(1, 1) + _b*stress(2, 2) - _strength;
}

RankTwoTensor
TensorMechanicsPlasticSimpleTester::dyieldFunction_dstress(const RankTwoTensor & /*stress*/, const Real & /*intnl*/) const
{
  RankTwoTensor df_dsig;
  df_dsig(1, 1) = _a;
  df_dsig(2, 2) = _b;
  return df_dsig;
}


Real
TensorMechanicsPlasticSimpleTester::dyieldFunction_dintnl(const RankTwoTensor & /*stress*/, const Real & /*intnl*/) const
{
  return 0.0;
}

RankTwoTensor
TensorMechanicsPlasticSimpleTester::flowPotential(const RankTwoTensor & /*stress*/, const Real & /*intnl*/) const
{
  RankTwoTensor df_dsig;
  df_dsig(1, 1) = _a;
  df_dsig(2, 2) = _b;
  return df_dsig;
}

RankFourTensor
TensorMechanicsPlasticSimpleTester::dflowPotential_dstress(const RankTwoTensor & /*stress*/, const Real & /*intnl*/) const
{
  return RankFourTensor();
}

RankTwoTensor
TensorMechanicsPlasticSimpleTester::dflowPotential_dintnl(const RankTwoTensor & /*stress*/, const Real & /*intnl*/) const
{
  return RankTwoTensor();
}
