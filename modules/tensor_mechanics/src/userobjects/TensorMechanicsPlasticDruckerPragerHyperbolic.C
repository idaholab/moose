//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TensorMechanicsPlasticDruckerPragerHyperbolic.h"
#include "RankFourTensor.h"
#include "libmesh/utility.h"

registerMooseObject("TensorMechanicsApp", TensorMechanicsPlasticDruckerPragerHyperbolic);

InputParameters
TensorMechanicsPlasticDruckerPragerHyperbolic::validParams()
{
  InputParameters params = TensorMechanicsPlasticDruckerPrager::validParams();
  params.addParam<bool>("use_custom_returnMap",
                        true,
                        "Whether to use the custom returnMap "
                        "algorithm.  Set to true if you are using "
                        "isotropic elasticity.");
  params.addParam<bool>("use_custom_cto",
                        true,
                        "Whether to use the custom consistent tangent "
                        "operator computations.  Set to true if you are "
                        "using isotropic elasticity.");
  params.addClassDescription("J2 plasticity, associative, with hardening");
  params.addRangeCheckedParam<Real>("smoother",
                                    0.0,
                                    "smoother>=0",
                                    "The cone vertex at J2=0 is smoothed.  The maximum mean "
                                    "stress possible, which is Cohesion*Cot(friction_angle) for "
                                    "smoother=0, becomes (Cohesion - "
                                    "smoother/3)*Cot(friction_angle).  This is a non-hardening "
                                    "parameter");
  params.addRangeCheckedParam<unsigned>(
      "max_iterations",
      40,
      "max_iterations>0",
      "Maximum iterations to use in the custom return map function");
  params.addClassDescription(
      "Non-associative Drucker Prager plasticity with hyperbolic smoothing of the cone tip.");
  return params;
}

TensorMechanicsPlasticDruckerPragerHyperbolic::TensorMechanicsPlasticDruckerPragerHyperbolic(
    const InputParameters & parameters)
  : TensorMechanicsPlasticDruckerPrager(parameters),
    _smoother2(Utility::pow<2>(getParam<Real>("smoother"))),
    _use_custom_returnMap(getParam<bool>("use_custom_returnMap")),
    _use_custom_cto(getParam<bool>("use_custom_cto")),
    _max_iters(getParam<unsigned>("max_iterations"))
{
}

Real
TensorMechanicsPlasticDruckerPragerHyperbolic::yieldFunction(const RankTwoTensor & stress,
                                                             Real intnl) const
{
  Real aaa;
  Real bbb;
  bothAB(intnl, aaa, bbb);
  return std::sqrt(stress.secondInvariant() + _smoother2) + stress.trace() * bbb - aaa;
}

RankTwoTensor
TensorMechanicsPlasticDruckerPragerHyperbolic::df_dsig(const RankTwoTensor & stress, Real bbb) const
{
  return 0.5 * stress.dsecondInvariant() / std::sqrt(stress.secondInvariant() + _smoother2) +
         stress.dtrace() * bbb;
}

RankFourTensor
TensorMechanicsPlasticDruckerPragerHyperbolic::dflowPotential_dstress(const RankTwoTensor & stress,
                                                                      Real /*intnl*/) const
{
  RankFourTensor dr_dstress;
  dr_dstress = 0.5 * stress.d2secondInvariant() / std::sqrt(stress.secondInvariant() + _smoother2);
  dr_dstress += -0.5 * 0.5 * stress.dsecondInvariant().outerProduct(stress.dsecondInvariant()) /
                std::pow(stress.secondInvariant() + _smoother2, 1.5);
  return dr_dstress;
}

std::string
TensorMechanicsPlasticDruckerPragerHyperbolic::modelName() const
{
  return "DruckerPragerHyperbolic";
}

bool
TensorMechanicsPlasticDruckerPragerHyperbolic::returnMap(const RankTwoTensor & trial_stress,
                                                         Real intnl_old,
                                                         const RankFourTensor & E_ijkl,
                                                         Real ep_plastic_tolerance,
                                                         RankTwoTensor & returned_stress,
                                                         Real & returned_intnl,
                                                         std::vector<Real> & dpm,
                                                         RankTwoTensor & delta_dp,
                                                         std::vector<Real> & yf,
                                                         bool & trial_stress_inadmissible) const
{
  if (!(_use_custom_returnMap))
    return TensorMechanicsPlasticModel::returnMap(trial_stress,
                                                  intnl_old,
                                                  E_ijkl,
                                                  ep_plastic_tolerance,
                                                  returned_stress,
                                                  returned_intnl,
                                                  dpm,
                                                  delta_dp,
                                                  yf,
                                                  trial_stress_inadmissible);

  yf.resize(1);

  yf[0] = yieldFunction(trial_stress, intnl_old);

  if (yf[0] < _f_tol)
  {
    // the trial_stress is admissible
    trial_stress_inadmissible = false;
    return true;
  }

  trial_stress_inadmissible = true;
  const Real mu = E_ijkl(0, 1, 0, 1);
  const Real lambda = E_ijkl(0, 0, 0, 0) - 2.0 * mu;
  const Real bulky = 3.0 * lambda + 2.0 * mu;
  const Real Tr_trial = trial_stress.trace();
  const Real J2trial = trial_stress.secondInvariant();

  // Perform a Newton-Raphson to find dpm when
  // residual = (1 + dpm*mu/ll)sqrt(ll^2 - s^2) - sqrt(J2trial) = 0, with s=smoother
  // with ll = sqrt(J2 + s^2) = aaa - bbb*Tr(stress) = aaa - bbb(Tr_trial - p*3*bulky*bbb_flow)
  Real aaa;
  Real daaa;
  Real bbb;
  Real dbbb;
  Real bbb_flow;
  Real dbbb_flow;
  Real ll;
  Real dll;
  Real residual;
  Real jac;
  dpm[0] = 0;
  unsigned int iter = 0;
  do
  {
    bothAB(intnl_old + dpm[0], aaa, bbb);
    dbothAB(intnl_old + dpm[0], daaa, dbbb);
    onlyB(intnl_old + dpm[0], dilation, bbb_flow);
    donlyB(intnl_old + dpm[0], dilation, dbbb_flow);
    ll = aaa - bbb * (Tr_trial - dpm[0] * bulky * 3 * bbb_flow);
    dll = daaa - dbbb * (Tr_trial - dpm[0] * bulky * 3 * bbb_flow) +
          bbb * bulky * 3 * (bbb_flow + dpm[0] * dbbb_flow);
    residual = bbb * (Tr_trial - dpm[0] * bulky * 3 * bbb_flow) - aaa +
               std::sqrt(J2trial / Utility::pow<2>(1 + dpm[0] * mu / ll) + _smoother2);
    jac = dbbb * (Tr_trial - dpm[0] * bulky * 3 * bbb_flow) -
          bbb * bulky * 3 * (bbb_flow + dpm[0] * dbbb_flow) - daaa +
          0.5 * J2trial * (-2.0) * (mu / ll - dpm[0] * mu * dll / ll / ll) /
              Utility::pow<3>(1 + dpm[0] * mu / ll) /
              std::sqrt(J2trial / Utility::pow<2>(1.0 + dpm[0] * mu / ll) + _smoother2);
    dpm[0] += -residual / jac;
    if (iter > _max_iters) // not converging
      return false;
    iter++;
  } while (residual * residual > _f_tol * _f_tol);

  // set the returned values
  yf[0] = 0;
  returned_intnl = intnl_old + dpm[0];

  bothAB(returned_intnl, aaa, bbb);
  onlyB(returned_intnl, dilation, bbb_flow);
  ll = aaa - bbb * (Tr_trial - dpm[0] * bulky * 3.0 * bbb_flow);
  returned_stress =
      trial_stress.deviatoric() / (1.0 + dpm[0] * mu / ll); // this is the deviatoric part only

  RankTwoTensor rij = 0.5 * returned_stress.deviatoric() /
                      ll; // this is the derivatoric part the flow potential only

  // form the returned stress and the full flow potential
  const Real returned_trace_over_3 = (aaa - ll) / bbb / 3.0;
  for (unsigned i = 0; i < 3; ++i)
  {
    returned_stress(i, i) += returned_trace_over_3;
    rij(i, i) += bbb_flow;
  }

  delta_dp = rij * dpm[0];

  return true;
}

RankFourTensor
TensorMechanicsPlasticDruckerPragerHyperbolic::consistentTangentOperator(
    const RankTwoTensor & trial_stress,
    Real intnl_old,
    const RankTwoTensor & stress,
    Real intnl,
    const RankFourTensor & E_ijkl,
    const std::vector<Real> & cumulative_pm) const
{
  if (!_use_custom_cto)
    return TensorMechanicsPlasticModel::consistentTangentOperator(
        trial_stress, intnl_old, stress, intnl, E_ijkl, cumulative_pm);

  const Real mu = E_ijkl(0, 1, 0, 1);
  const Real la = E_ijkl(0, 0, 0, 0) - 2.0 * mu;
  const Real bulky = 3.0 * la + 2.0 * mu;
  Real bbb;
  onlyB(intnl, friction, bbb);
  Real bbb_flow;
  onlyB(intnl, dilation, bbb_flow);
  Real dbbb_flow;
  donlyB(intnl, dilation, dbbb_flow);
  const Real bbb_flow_mod = bbb_flow + cumulative_pm[0] * dbbb_flow;
  const Real J2 = stress.secondInvariant();
  const RankTwoTensor sij = stress.deviatoric();
  const Real sq = std::sqrt(J2 + _smoother2);

  const Real one_over_h =
      1.0 / (-dyieldFunction_dintnl(stress, intnl) + mu * J2 / Utility::pow<2>(sq) +
             3.0 * bbb * bbb_flow_mod * bulky); // correct up to hard

  const RankTwoTensor df_dsig_timesE =
      mu * sij / sq + bulky * bbb * RankTwoTensor(RankTwoTensor::initIdentity); // correct
  const RankTwoTensor rmod_timesE =
      mu * sij / sq +
      bulky * bbb_flow_mod * RankTwoTensor(RankTwoTensor::initIdentity); // correct up to hard

  const RankFourTensor coeff_ep =
      E_ijkl - one_over_h * rmod_timesE.outerProduct(df_dsig_timesE); // correct

  const RankFourTensor dr_dsig_timesE = -0.5 * mu * sij.outerProduct(sij) / Utility::pow<3>(sq) +
                                        mu * stress.d2secondInvariant() / sq; // correct
  const RankTwoTensor df_dsig_E_dr_dsig =
      0.5 * mu * _smoother2 * sij / Utility::pow<4>(sq); // correct

  const RankFourTensor coeff_si =
      RankFourTensor(RankFourTensor::initIdentitySymmetricFour) +
      cumulative_pm[0] *
          (dr_dsig_timesE - one_over_h * rmod_timesE.outerProduct(df_dsig_E_dr_dsig));

  RankFourTensor s_inv;
  try
  {
    s_inv = coeff_si.invSymm();
  }
  catch (MooseException & e)
  {
    return coeff_ep; // when coeff_si is singular return the "linear" tangent operator
  }

  return s_inv * coeff_ep;
}

bool
TensorMechanicsPlasticDruckerPragerHyperbolic::useCustomReturnMap() const
{
  return _use_custom_returnMap;
}

bool
TensorMechanicsPlasticDruckerPragerHyperbolic::useCustomCTO() const
{
  return _use_custom_cto;
}
