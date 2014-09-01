//  Plastic Model base class.
//
#include "TensorMechanicsPlasticModel.h"

template<>
InputParameters validParams<TensorMechanicsPlasticModel>()
{
  InputParameters params = validParams<GeneralUserObject>();
  params.addRequiredRangeCheckedParam<Real>("yield_function_tolerance", "yield_function_tolerance>0", "If the yield function is less than this amount, the (stress, internal parameter) are deemed admissible.");
  params.addRequiredRangeCheckedParam<Real>("internal_constraint_tolerance", "internal_constraint_tolerance>0", "The Newton-Raphson process is only deemed converged if the internal constraint is less than this.");
  params.addClassDescription("Plastic Model base class.  Override the virtual functions in your class");
  return params;
}

TensorMechanicsPlasticModel::TensorMechanicsPlasticModel(const std::string & name, InputParameters parameters) :
  GeneralUserObject(name, parameters),
  _f_tol(getParam<Real>("yield_function_tolerance")),
  _ic_tol(getParam<Real>("internal_constraint_tolerance"))
{}

void
TensorMechanicsPlasticModel::initialize()
{}

void
TensorMechanicsPlasticModel::execute()
{}

void TensorMechanicsPlasticModel::finalize()
{}

RankTwoTensor
TensorMechanicsPlasticModel::dyieldFunction_dstress(const RankTwoTensor & /*stress*/, const Real & /*intnl*/) const
{
  return RankTwoTensor();
}

RankTwoTensor
TensorMechanicsPlasticModel::flowPotential(const RankTwoTensor & /*stress*/, const Real & /*intnl*/) const
{
  return RankTwoTensor();
}

RankFourTensor
TensorMechanicsPlasticModel::dflowPotential_dstress(const RankTwoTensor & /*stress*/, const Real & /*intnl*/) const
{
  return RankFourTensor();
}

RankTwoTensor
TensorMechanicsPlasticModel::dflowPotential_dintnl(const RankTwoTensor & /*stress*/, const Real & /*intnl*/) const
{
  return RankTwoTensor();
}

Real
TensorMechanicsPlasticModel::hardPotential(const RankTwoTensor & /*stress*/, const Real & /*intnl*/) const
{
  return -1.0;
}

RankTwoTensor
TensorMechanicsPlasticModel::dhardPotential_dstress(const RankTwoTensor & /*stress*/, const Real & /*intnl*/) const
{
  return RankTwoTensor();
}

Real
TensorMechanicsPlasticModel::dhardPotential_dintnl(const RankTwoTensor & /*stress*/, const Real & /*intnl*/) const
{
  return 0.0;
}

