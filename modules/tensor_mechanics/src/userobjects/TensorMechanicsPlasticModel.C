/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
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

unsigned
TensorMechanicsPlasticModel::numberSurfaces() const
{
  return 1;
}



Real
TensorMechanicsPlasticModel::yieldFunction(const RankTwoTensor & /*stress*/, const Real & /*intnl*/) const
{
  return 0.0;
}
void
TensorMechanicsPlasticModel::yieldFunctionV(const RankTwoTensor & stress, const Real & intnl, std::vector<Real> & f) const
{
  f.assign(1, yieldFunction(stress, intnl));
}



RankTwoTensor
TensorMechanicsPlasticModel::dyieldFunction_dstress(const RankTwoTensor & /*stress*/, const Real & /*intnl*/) const
{
  return RankTwoTensor();
}
void
TensorMechanicsPlasticModel::dyieldFunction_dstressV(const RankTwoTensor & stress, const Real & intnl, std::vector<RankTwoTensor> & df_dstress) const
{
  df_dstress.assign(1, dyieldFunction_dstress(stress, intnl));
}


Real
TensorMechanicsPlasticModel::dyieldFunction_dintnl(const RankTwoTensor & /*stress*/, const Real & /*intnl*/) const
{
  return 0.0;
}
void
TensorMechanicsPlasticModel::dyieldFunction_dintnlV(const RankTwoTensor & stress, const Real & intnl, std::vector<Real> & df_dintnl) const
{
  return df_dintnl.assign(1, dyieldFunction_dintnl(stress, intnl));
}



RankTwoTensor
TensorMechanicsPlasticModel::flowPotential(const RankTwoTensor & /*stress*/, const Real & /*intnl*/) const
{
  return RankTwoTensor();
}
void
TensorMechanicsPlasticModel::flowPotentialV(const RankTwoTensor & stress, const Real & intnl, std::vector<RankTwoTensor> & r) const
{
  return r.assign(1, flowPotential(stress, intnl));
}


RankFourTensor
TensorMechanicsPlasticModel::dflowPotential_dstress(const RankTwoTensor & /*stress*/, const Real & /*intnl*/) const
{
  return RankFourTensor();
}
void
TensorMechanicsPlasticModel::dflowPotential_dstressV(const RankTwoTensor & stress, const Real & intnl, std::vector<RankFourTensor> & dr_dstress) const
{
  return dr_dstress.assign(1, dflowPotential_dstress(stress, intnl));
}



RankTwoTensor
TensorMechanicsPlasticModel::dflowPotential_dintnl(const RankTwoTensor & /*stress*/, const Real & /*intnl*/) const
{
  return RankTwoTensor();
}
void
TensorMechanicsPlasticModel::dflowPotential_dintnlV(const RankTwoTensor & stress, const Real & intnl, std::vector<RankTwoTensor> & dr_dintnl) const
{
  return dr_dintnl.assign(1, dflowPotential_dintnl(stress, intnl));
}



Real
TensorMechanicsPlasticModel::hardPotential(const RankTwoTensor & /*stress*/, const Real & /*intnl*/) const
{
  return -1.0;
}
void
TensorMechanicsPlasticModel::hardPotentialV(const RankTwoTensor & stress, const Real & intnl, std::vector<Real> & h) const
{
  h.assign(numberSurfaces(), hardPotential(stress, intnl));
}



RankTwoTensor
TensorMechanicsPlasticModel::dhardPotential_dstress(const RankTwoTensor & /*stress*/, const Real & /*intnl*/) const
{
  return RankTwoTensor();
}
void
TensorMechanicsPlasticModel::dhardPotential_dstressV(const RankTwoTensor & stress, const Real & intnl, std::vector<RankTwoTensor> & dh_dstress) const
{
  dh_dstress.assign(numberSurfaces(), dhardPotential_dstress(stress, intnl));
}


Real
TensorMechanicsPlasticModel::dhardPotential_dintnl(const RankTwoTensor & /*stress*/, const Real & /*intnl*/) const
{
  return 0.0;
}
void
TensorMechanicsPlasticModel::dhardPotential_dintnlV(const RankTwoTensor & stress, const Real & intnl, std::vector<Real> & dh_dintnl) const
{
  dh_dintnl.resize(numberSurfaces(), dhardPotential_dintnl(stress, intnl));
}


void
TensorMechanicsPlasticModel::activeConstraints(const std::vector<Real> & f, const RankTwoTensor & /*stress*/, const Real & /*intnl*/, const RankFourTensor & /*Eijkl*/, std::vector<bool> & act, RankTwoTensor & /*returned_stress*/) const
{
  mooseAssert(f.size() == numberSurfaces(), "f incorrectly sized at " << f.size() << " in activeConstraints");
  act.resize(numberSurfaces());
  for (unsigned surface = 0 ; surface < numberSurfaces() ; ++surface)
    act[surface] = (f[surface] > _f_tol);
}

std::string
TensorMechanicsPlasticModel::modelName() const
{
  return "None";
}
