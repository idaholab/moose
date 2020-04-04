//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

//  Plastic Model base class.
//
#include "TensorMechanicsPlasticModel.h"
#include "RankFourTensor.h"

InputParameters
TensorMechanicsPlasticModel::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addRequiredRangeCheckedParam<Real>("yield_function_tolerance",
                                            "yield_function_tolerance>0",
                                            "If the yield function is less than this amount, the "
                                            "(stress, internal parameter) are deemed admissible.");
  params.addRequiredRangeCheckedParam<Real>("internal_constraint_tolerance",
                                            "internal_constraint_tolerance>0",
                                            "The Newton-Raphson process is only deemed converged "
                                            "if the internal constraint is less than this.");
  params.addClassDescription(
      "Plastic Model base class.  Override the virtual functions in your class");
  return params;
}

TensorMechanicsPlasticModel::TensorMechanicsPlasticModel(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    _f_tol(getParam<Real>("yield_function_tolerance")),
    _ic_tol(getParam<Real>("internal_constraint_tolerance"))
{
}

void
TensorMechanicsPlasticModel::initialize()
{
}

void
TensorMechanicsPlasticModel::execute()
{
}

void
TensorMechanicsPlasticModel::finalize()
{
}

unsigned
TensorMechanicsPlasticModel::numberSurfaces() const
{
  return 1;
}

Real
TensorMechanicsPlasticModel::yieldFunction(const RankTwoTensor & /*stress*/, Real /*intnl*/) const
{
  return 0.0;
}

void
TensorMechanicsPlasticModel::yieldFunctionV(const RankTwoTensor & stress,
                                            Real intnl,
                                            std::vector<Real> & f) const
{
  f.assign(1, yieldFunction(stress, intnl));
}

RankTwoTensor
TensorMechanicsPlasticModel::dyieldFunction_dstress(const RankTwoTensor & /*stress*/,
                                                    Real /*intnl*/) const
{
  return RankTwoTensor();
}

void
TensorMechanicsPlasticModel::dyieldFunction_dstressV(const RankTwoTensor & stress,
                                                     Real intnl,
                                                     std::vector<RankTwoTensor> & df_dstress) const
{
  df_dstress.assign(1, dyieldFunction_dstress(stress, intnl));
}

Real
TensorMechanicsPlasticModel::dyieldFunction_dintnl(const RankTwoTensor & /*stress*/,
                                                   Real /*intnl*/) const
{
  return 0.0;
}
void
TensorMechanicsPlasticModel::dyieldFunction_dintnlV(const RankTwoTensor & stress,
                                                    Real intnl,
                                                    std::vector<Real> & df_dintnl) const
{
  return df_dintnl.assign(1, dyieldFunction_dintnl(stress, intnl));
}

RankTwoTensor
TensorMechanicsPlasticModel::flowPotential(const RankTwoTensor & /*stress*/, Real /*intnl*/) const
{
  return RankTwoTensor();
}
void
TensorMechanicsPlasticModel::flowPotentialV(const RankTwoTensor & stress,
                                            Real intnl,
                                            std::vector<RankTwoTensor> & r) const
{
  return r.assign(1, flowPotential(stress, intnl));
}

RankFourTensor
TensorMechanicsPlasticModel::dflowPotential_dstress(const RankTwoTensor & /*stress*/,
                                                    Real /*intnl*/) const
{
  return RankFourTensor();
}
void
TensorMechanicsPlasticModel::dflowPotential_dstressV(const RankTwoTensor & stress,
                                                     Real intnl,
                                                     std::vector<RankFourTensor> & dr_dstress) const
{
  return dr_dstress.assign(1, dflowPotential_dstress(stress, intnl));
}

RankTwoTensor
TensorMechanicsPlasticModel::dflowPotential_dintnl(const RankTwoTensor & /*stress*/,
                                                   Real /*intnl*/) const
{
  return RankTwoTensor();
}
void
TensorMechanicsPlasticModel::dflowPotential_dintnlV(const RankTwoTensor & stress,
                                                    Real intnl,
                                                    std::vector<RankTwoTensor> & dr_dintnl) const
{
  return dr_dintnl.assign(1, dflowPotential_dintnl(stress, intnl));
}

Real
TensorMechanicsPlasticModel::hardPotential(const RankTwoTensor & /*stress*/, Real /*intnl*/) const
{
  return -1.0;
}
void
TensorMechanicsPlasticModel::hardPotentialV(const RankTwoTensor & stress,
                                            Real intnl,
                                            std::vector<Real> & h) const
{
  h.assign(numberSurfaces(), hardPotential(stress, intnl));
}

RankTwoTensor
TensorMechanicsPlasticModel::dhardPotential_dstress(const RankTwoTensor & /*stress*/,
                                                    Real /*intnl*/) const
{
  return RankTwoTensor();
}
void
TensorMechanicsPlasticModel::dhardPotential_dstressV(const RankTwoTensor & stress,
                                                     Real intnl,
                                                     std::vector<RankTwoTensor> & dh_dstress) const
{
  dh_dstress.assign(numberSurfaces(), dhardPotential_dstress(stress, intnl));
}

Real
TensorMechanicsPlasticModel::dhardPotential_dintnl(const RankTwoTensor & /*stress*/,
                                                   Real /*intnl*/) const
{
  return 0.0;
}
void
TensorMechanicsPlasticModel::dhardPotential_dintnlV(const RankTwoTensor & stress,
                                                    Real intnl,
                                                    std::vector<Real> & dh_dintnl) const
{
  dh_dintnl.resize(numberSurfaces(), dhardPotential_dintnl(stress, intnl));
}

void
TensorMechanicsPlasticModel::activeConstraints(const std::vector<Real> & f,
                                               const RankTwoTensor & /*stress*/,
                                               Real /*intnl*/,
                                               const RankFourTensor & /*Eijkl*/,
                                               std::vector<bool> & act,
                                               RankTwoTensor & /*returned_stress*/) const
{
  mooseAssert(f.size() == numberSurfaces(),
              "f incorrectly sized at " << f.size() << " in activeConstraints");
  act.resize(numberSurfaces());
  for (unsigned surface = 0; surface < numberSurfaces(); ++surface)
    act[surface] = (f[surface] > _f_tol);
}

std::string
TensorMechanicsPlasticModel::modelName() const
{
  return "None";
}

bool
TensorMechanicsPlasticModel::useCustomReturnMap() const
{
  return false;
}

bool
TensorMechanicsPlasticModel::useCustomCTO() const
{
  return false;
}

bool
TensorMechanicsPlasticModel::returnMap(const RankTwoTensor & trial_stress,
                                       Real intnl_old,
                                       const RankFourTensor & /*E_ijkl*/,
                                       Real /*ep_plastic_tolerance*/,
                                       RankTwoTensor & /*returned_stress*/,
                                       Real & /*returned_intnl*/,
                                       std::vector<Real> & /*dpm*/,
                                       RankTwoTensor & /*delta_dp*/,
                                       std::vector<Real> & yf,
                                       bool & trial_stress_inadmissible) const
{
  trial_stress_inadmissible = false;
  yieldFunctionV(trial_stress, intnl_old, yf);

  for (unsigned sf = 0; sf < numberSurfaces(); ++sf)
    if (yf[sf] > _f_tol)
      trial_stress_inadmissible = true;

  // example of checking Kuhn-Tucker
  std::vector<Real> dpm(numberSurfaces(), 0);
  for (unsigned sf = 0; sf < numberSurfaces(); ++sf)
    if (!KuhnTuckerSingleSurface(yf[sf], dpm[sf], 0))
      return false;
  return true;
}

bool
TensorMechanicsPlasticModel::KuhnTuckerSingleSurface(Real yf, Real dpm, Real dpm_tol) const
{
  return (dpm == 0 && yf <= _f_tol) || (dpm > -dpm_tol && yf <= _f_tol && yf >= -_f_tol);
}

RankFourTensor
TensorMechanicsPlasticModel::consistentTangentOperator(
    const RankTwoTensor & /*trial_stress*/,
    Real /*intnl_old*/,
    const RankTwoTensor & /*stress*/,
    Real /*intnl*/,
    const RankFourTensor & E_ijkl,
    const std::vector<Real> & /*cumulative_pm*/) const
{
  return E_ijkl;
}
