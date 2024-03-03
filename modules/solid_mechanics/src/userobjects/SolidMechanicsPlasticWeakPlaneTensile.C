//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SolidMechanicsPlasticWeakPlaneTensile.h"
#include "RankFourTensor.h"

registerMooseObject("SolidMechanicsApp", SolidMechanicsPlasticWeakPlaneTensile);
registerMooseObjectRenamed("SolidMechanicsApp",
                           TensorMechanicsPlasticWeakPlaneTensile,
                           "01/01/2025 00:00",
                           SolidMechanicsPlasticWeakPlaneTensile);

InputParameters
SolidMechanicsPlasticWeakPlaneTensile::validParams()
{
  InputParameters params = SolidMechanicsPlasticModel::validParams();
  params.addParam<Real>("stress_coefficient",
                        1.0,
                        "The yield function is stress_coefficient * stress_zz - tensile_strength");
  params.addRequiredParam<UserObjectName>("tensile_strength",
                                          "A SolidMechanicsHardening "
                                          "UserObject that defines hardening "
                                          "of the weak-plane tensile strength");
  params.addClassDescription("Associative weak-plane tensile plasticity with hardening/softening");

  return params;
}

SolidMechanicsPlasticWeakPlaneTensile::SolidMechanicsPlasticWeakPlaneTensile(
    const InputParameters & parameters)
  : SolidMechanicsPlasticModel(parameters),
    _a(getParam<Real>("stress_coefficient")),
    _strength(getUserObject<SolidMechanicsHardeningModel>("tensile_strength"))
{
  // cannot check the following for all values of strength, but this is a start
  if (_strength.value(0) < 0)
    mooseError("Weak plane tensile strength must not be negative");
}

Real
SolidMechanicsPlasticWeakPlaneTensile::yieldFunction(const RankTwoTensor & stress, Real intnl) const
{
  return _a * stress(2, 2) - tensile_strength(intnl);
}

RankTwoTensor
SolidMechanicsPlasticWeakPlaneTensile::dyieldFunction_dstress(const RankTwoTensor & /*stress*/,
                                                              Real /*intnl*/) const
{
  RankTwoTensor df_dsig;
  df_dsig(2, 2) = _a;
  return df_dsig;
}

Real
SolidMechanicsPlasticWeakPlaneTensile::dyieldFunction_dintnl(const RankTwoTensor & /*stress*/,
                                                             Real intnl) const
{
  return -dtensile_strength(intnl);
}

RankTwoTensor
SolidMechanicsPlasticWeakPlaneTensile::flowPotential(const RankTwoTensor & /*stress*/,
                                                     Real /*intnl*/) const
{
  RankTwoTensor df_dsig;
  df_dsig(2, 2) = _a;
  return df_dsig;
}

RankFourTensor
SolidMechanicsPlasticWeakPlaneTensile::dflowPotential_dstress(const RankTwoTensor & /*stress*/,
                                                              Real /*intnl*/) const
{
  return RankFourTensor();
}

RankTwoTensor
SolidMechanicsPlasticWeakPlaneTensile::dflowPotential_dintnl(const RankTwoTensor & /*stress*/,
                                                             Real /*intnl*/) const
{
  return RankTwoTensor();
}

Real
SolidMechanicsPlasticWeakPlaneTensile::tensile_strength(const Real internal_param) const
{
  return _strength.value(internal_param);
}

Real
SolidMechanicsPlasticWeakPlaneTensile::dtensile_strength(const Real internal_param) const
{
  return _strength.derivative(internal_param);
}

void
SolidMechanicsPlasticWeakPlaneTensile::activeConstraints(const std::vector<Real> & f,
                                                         const RankTwoTensor & stress,
                                                         Real intnl,
                                                         const RankFourTensor & Eijkl,
                                                         std::vector<bool> & act,
                                                         RankTwoTensor & returned_stress) const
{
  act.assign(1, false);

  if (f[0] <= _f_tol)
  {
    returned_stress = stress;
    return;
  }

  Real str = tensile_strength(intnl);

  RankTwoTensor n; // flow direction
  for (unsigned i = 0; i < 3; ++i)
    for (unsigned j = 0; j < 3; ++j)
      n(i, j) = _a * Eijkl(i, j, 2, 2);

  // returned_stress = _a * stress - alpha*n
  // where alpha = (_a * stress(2, 2) - str)/n(2, 2)
  Real alpha = (_a * stress(2, 2) - str) / n(2, 2);

  for (unsigned i = 0; i < 3; ++i)
    for (unsigned j = 0; j < 3; ++j)
      returned_stress(i, j) = _a * stress(i, j) - alpha * n(i, j);

  act[0] = true;
}

std::string
SolidMechanicsPlasticWeakPlaneTensile::modelName() const
{
  return "WeakPlaneTensile";
}
