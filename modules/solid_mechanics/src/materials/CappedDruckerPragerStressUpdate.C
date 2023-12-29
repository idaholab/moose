//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CappedDruckerPragerStressUpdate.h"

#include "libmesh/utility.h"

registerMooseObject("TensorMechanicsApp", CappedDruckerPragerStressUpdate);

InputParameters
CappedDruckerPragerStressUpdate::validParams()
{
  InputParameters params = TwoParameterPlasticityStressUpdate::validParams();
  params.addClassDescription("Capped Drucker-Prager plasticity stress calculator");
  params.addRequiredParam<UserObjectName>(
      "DP_model",
      "A TensorMechanicsPlasticDruckerPrager UserObject that defines the "
      "Drucker-Prager parameters (cohesion, friction angle and dilation angle)");
  params.addRequiredParam<UserObjectName>(
      "tensile_strength",
      "A TensorMechanicsHardening UserObject that defines hardening of the "
      "tensile strength.  In physical situations this is positive (and always "
      "must be greater than negative compressive-strength.");
  params.addRequiredParam<UserObjectName>(
      "compressive_strength",
      "A TensorMechanicsHardening UserObject that defines hardening of the "
      "compressive strength.  In physical situations this is positive.");
  params.addRequiredRangeCheckedParam<Real>(
      "tip_smoother",
      "tip_smoother>=0",
      "The cone vertex at J2 = 0 will be smoothed by the given "
      "amount.  Typical value is 0.1*cohesion");
  params.addParam<bool>("perfect_guess",
                        true,
                        "Provide a guess to the Newton-Raphson procedure "
                        "that is the result from perfect plasticity.  With "
                        "severe hardening/softening this may be "
                        "suboptimal.");
  params.addParam<bool>("small_dilation",
                        true,
                        "If true, and if the trial stress exceeds the "
                        "tensile strength, then the user guarantees that "
                        "the returned stress will be independent of the "
                        "compressive strength.");
  return params;
}

CappedDruckerPragerStressUpdate::CappedDruckerPragerStressUpdate(const InputParameters & parameters)
  : TwoParameterPlasticityStressUpdate(parameters, 3, 2),
    _dp(getUserObject<TensorMechanicsPlasticDruckerPrager>("DP_model")),
    _tstrength(getUserObject<TensorMechanicsHardeningModel>("tensile_strength")),
    _cstrength(getUserObject<TensorMechanicsHardeningModel>("compressive_strength")),
    _small_smoother2(Utility::pow<2>(getParam<Real>("tip_smoother"))),
    _perfect_guess(getParam<bool>("perfect_guess")),
    _stress_return_type(StressReturnType::nothing_special),
    _small_dilation(getParam<bool>("small_dilation")),
    _in_q_trial(0.0)
{
  // With arbitary UserObjects, it is impossible to check everything,
  // but this will catch the common errors
  if (_tstrength.value(0) + _cstrength.value(0) <= _smoothing_tol)
    mooseError("CappedDruckerPragerStressUpdate: Tensile strength plus compressive strength must "
               "be greater than smoothing_tol");
}

void
CappedDruckerPragerStressUpdate::initializeReturnProcess()
{
  _stress_return_type = StressReturnType::nothing_special;
}

void
CappedDruckerPragerStressUpdate::finalizeReturnProcess(const RankTwoTensor & /*rotation_increment*/)
{
  _stress_return_type = StressReturnType::nothing_special;
}

void
CappedDruckerPragerStressUpdate::preReturnMap(Real /*p_trial*/,
                                              Real q_trial,
                                              const RankTwoTensor & /*stress_trial*/,
                                              const std::vector<Real> & /*intnl_old*/,
                                              const std::vector<Real> & yf,
                                              const RankFourTensor & /*Eijkl*/)
{
  // If it's obvious, then simplify the return-type
  if (yf[2] >= 0)
    _stress_return_type = StressReturnType::no_tension;
  else if (_small_dilation && yf[1] >= 0)
    _stress_return_type = StressReturnType::no_compression;

  _in_q_trial = q_trial;
}

void
CappedDruckerPragerStressUpdate::computePQ(const RankTwoTensor & stress, Real & p, Real & q) const
{
  p = stress.trace();
  q = std::sqrt(stress.secondInvariant());
}

void
CappedDruckerPragerStressUpdate::setEppEqq(const RankFourTensor & Eijkl,
                                           Real & Epp,
                                           Real & Eqq) const
{
  Epp = Eijkl.sum3x3();
  Eqq = Eijkl(0, 1, 0, 1);
}

void
CappedDruckerPragerStressUpdate::setIntnlDerivatives(Real /*p_trial*/,
                                                     Real q_trial,
                                                     Real /*p*/,
                                                     Real q,
                                                     const std::vector<Real> & intnl,
                                                     std::vector<std::vector<Real>> & dintnl) const
{
  Real tanpsi;
  _dp.onlyB(intnl[0], _dp.dilation, tanpsi);
  Real dtanpsi;
  _dp.donlyB(intnl[0], _dp.dilation, dtanpsi);
  dintnl[0][0] = 0.0;
  dintnl[0][1] = -1.0 / _Eqq;
  dintnl[1][0] = -1.0 / _Epp;
  dintnl[1][1] = tanpsi / _Eqq - (q_trial - q) * dtanpsi * dintnl[0][1] / _Eqq;
}

void
CappedDruckerPragerStressUpdate::yieldFunctionValues(Real p,
                                                     Real q,
                                                     const std::vector<Real> & intnl,
                                                     std::vector<Real> & yf) const
{
  Real aaa;
  Real bbb;
  _dp.bothAB(intnl[0], aaa, bbb);
  yf[0] = std::sqrt(Utility::pow<2>(q) + _small_smoother2) + p * bbb - aaa;

  if (_stress_return_type == StressReturnType::no_tension)
    yf[1] = std::numeric_limits<Real>::lowest();
  else
    yf[1] = p - _tstrength.value(intnl[1]);

  if (_stress_return_type == StressReturnType::no_compression)
    yf[2] = std::numeric_limits<Real>::lowest();
  else
    yf[2] = -p - _cstrength.value(intnl[1]);
}

void
CappedDruckerPragerStressUpdate::computeAllQ(Real p,
                                             Real q,
                                             const std::vector<Real> & intnl,
                                             std::vector<yieldAndFlow> & all_q) const
{
  Real aaa;
  Real bbb;
  _dp.bothAB(intnl[0], aaa, bbb);
  Real daaa;
  Real dbbb;
  _dp.dbothAB(intnl[0], daaa, dbbb);
  Real tanpsi;
  _dp.onlyB(intnl[0], _dp.dilation, tanpsi);
  Real dtanpsi;
  _dp.donlyB(intnl[0], _dp.dilation, dtanpsi);

  // yield function values
  all_q[0].f = std::sqrt(Utility::pow<2>(q) + _small_smoother2) + p * bbb - aaa;
  if (_stress_return_type == StressReturnType::no_tension)
    all_q[1].f = std::numeric_limits<Real>::lowest();
  else
    all_q[1].f = p - _tstrength.value(intnl[1]);
  if (_stress_return_type == StressReturnType::no_compression)
    all_q[2].f = std::numeric_limits<Real>::lowest();
  else
    all_q[2].f = -p - _cstrength.value(intnl[1]);

  // d(yield Function)/d(p, q)
  // derivatives wrt p
  all_q[0].df[0] = bbb;
  all_q[1].df[0] = 1.0;
  all_q[2].df[0] = -1.0;

  // derivatives wrt q
  if (_small_smoother2 == 0.0)
    all_q[0].df[1] = 1.0;
  else
    all_q[0].df[1] = q / std::sqrt(Utility::pow<2>(q) + _small_smoother2);
  all_q[1].df[1] = 0.0;
  all_q[2].df[1] = 0.0;

  // d(yield Function)/d(intnl)
  // derivatives wrt intnl[0] (shear plastic strain)
  all_q[0].df_di[0] = p * dbbb - daaa;
  all_q[1].df_di[0] = 0.0;
  all_q[2].df_di[0] = 0.0;
  // derivatives wrt intnl[q] (tensile plastic strain)
  all_q[0].df_di[1] = 0.0;
  all_q[1].df_di[1] = -_tstrength.derivative(intnl[1]);
  all_q[2].df_di[1] = -_cstrength.derivative(intnl[1]);

  // d(flowPotential)/d(p, q)
  // derivatives wrt p
  all_q[0].dg[0] = tanpsi;
  all_q[1].dg[0] = 1.0;
  all_q[2].dg[0] = -1.0;
  // derivatives wrt q
  if (_small_smoother2 == 0.0)
    all_q[0].dg[1] = 1.0;
  else
    all_q[0].dg[1] = q / std::sqrt(Utility::pow<2>(q) + _small_smoother2);
  all_q[1].dg[1] = 0.0;
  all_q[2].dg[1] = 0.0;

  // d2(flowPotential)/d(p, q)/d(intnl)
  // d(dg/dp)/dintnl[0]
  all_q[0].d2g_di[0][0] = dtanpsi;
  all_q[1].d2g_di[0][0] = 0.0;
  all_q[2].d2g_di[0][0] = 0.0;
  // d(dg/dp)/dintnl[1]
  all_q[0].d2g_di[0][1] = 0.0;
  all_q[1].d2g_di[0][1] = 0.0;
  all_q[2].d2g_di[0][1] = 0.0;
  // d(dg/dq)/dintnl[0]
  all_q[0].d2g_di[1][0] = 0.0;
  all_q[1].d2g_di[1][0] = 0.0;
  all_q[2].d2g_di[1][0] = 0.0;
  // d(dg/dq)/dintnl[1]
  all_q[0].d2g_di[1][1] = 0.0;
  all_q[1].d2g_di[1][1] = 0.0;
  all_q[2].d2g_di[1][1] = 0.0;

  // d2(flowPotential)/d(p, q)/d(p, q)
  // d(dg/dp)/dp
  all_q[0].d2g[0][0] = 0.0;
  all_q[1].d2g[0][0] = 0.0;
  all_q[2].d2g[0][0] = 0.0;
  // d(dg/dp)/dq
  all_q[0].d2g[0][1] = 0.0;
  all_q[1].d2g[0][1] = 0.0;
  all_q[2].d2g[0][1] = 0.0;
  // d(dg/dq)/dp
  all_q[0].d2g[1][0] = 0.0;
  all_q[1].d2g[1][0] = 0.0;
  all_q[2].d2g[1][0] = 0.0;
  // d(dg/dq)/dq
  if (_small_smoother2 == 0.0)
    all_q[0].d2g[1][1] = 0.0;
  else
    all_q[0].d2g[1][1] = _small_smoother2 / std::pow(Utility::pow<2>(q) + _small_smoother2, 1.5);
  all_q[1].d2g[1][1] = 0.0;
  all_q[2].d2g[1][1] = 0.0;
}

void
CappedDruckerPragerStressUpdate::initializeVars(Real p_trial,
                                                Real q_trial,
                                                const std::vector<Real> & intnl_old,
                                                Real & p,
                                                Real & q,
                                                Real & gaE,
                                                std::vector<Real> & intnl) const
{
  if (!_perfect_guess)
  {
    p = p_trial;
    q = q_trial;
    gaE = 0.0;
  }
  else
  {
    Real coh;
    Real tanphi;
    _dp.bothAB(intnl[0], coh, tanphi);
    Real tanpsi;
    _dp.onlyB(intnl_old[0], _dp.dilation, tanpsi);
    const Real tens = _tstrength.value(intnl_old[1]);
    const Real comp = _cstrength.value(intnl_old[1]);
    const Real q_at_T = coh - tens * tanphi;
    const Real q_at_C = coh + comp * tanphi;

    if ((p_trial >= tens) && (q_trial <= q_at_T))
    {
      // pure tensile failure
      p = tens;
      q = q_trial;
      gaE = p_trial - p;
    }
    else if ((p_trial <= -comp) && (q_trial <= q_at_C))
    {
      // pure compressive failure
      p = -comp;
      q = q_trial;
      gaE = p - p_trial;
    }
    else
    {
      // shear failure or a mixture
      // Calculate ga assuming a pure shear failure
      const Real ga = (q_trial + p_trial * tanphi - coh) / (_Eqq + _Epp * tanphi * tanpsi);
      if (ga <= 0 && p_trial <= tens && p_trial >= -comp)
      {
        // very close to one of the rounded corners:  there is no advantage to guessing the
        // solution, so:
        p = p_trial;
        q = q_trial;
        gaE = 0.0;
      }
      else
      {
        q = q_trial - _Eqq * ga;
        if (q <= 0.0 && q_at_T <= 0.0)
        {
          // user has set tensile strength so large that it is irrelevant: return to the tip of the
          // shear cone
          q = 0.0;
          p = coh / tanphi;
          gaE = (p_trial - p) / tanpsi; // just a guess, based on the angle to the corner
        }
        else if (q <= q_at_T)
        {
          // pure shear is incorrect: mixture of tension and shear is correct
          q = q_at_T;
          p = tens;
          gaE = (p_trial - p) / tanpsi; // just a guess, based on the angle to the corner
        }
        else if (q >= q_at_C)
        {
          // pure shear is incorrect: mixture of compression and shear is correct
          q = q_at_C;
          p = -comp;
          if (p - p_trial < _Epp * tanpsi * (q_trial - q) / _Eqq)
            // trial point is sitting almost directly above corner
            gaE = (q_trial - q) * _Epp / _Eqq;
          else
            // trial point is sitting to the left of the corner
            gaE = (p - p_trial) / tanpsi;
        }
        else
        {
          // pure shear was correct
          p = p_trial - _Epp * ga * tanpsi;
          gaE = ga * _Epp;
        }
      }
    }
  }
  setIntnlValues(p_trial, q_trial, p, q, intnl_old, intnl);
}

void
CappedDruckerPragerStressUpdate::setIntnlValues(Real p_trial,
                                                Real q_trial,
                                                Real p,
                                                Real q,
                                                const std::vector<Real> & intnl_old,
                                                std::vector<Real> & intnl) const
{
  intnl[0] = intnl_old[0] + (q_trial - q) / _Eqq;
  Real tanpsi;
  _dp.onlyB(intnl[0], _dp.dilation, tanpsi);
  intnl[1] = intnl_old[1] + (p_trial - p) / _Epp - (q_trial - q) * tanpsi / _Eqq;
}

void
CappedDruckerPragerStressUpdate::setStressAfterReturn(const RankTwoTensor & stress_trial,
                                                      Real p_ok,
                                                      Real q_ok,
                                                      Real /*gaE*/,
                                                      const std::vector<Real> & /*intnl*/,
                                                      const yieldAndFlow & /*smoothed_q*/,
                                                      const RankFourTensor & /*Eijkl*/,
                                                      RankTwoTensor & stress) const
{
  // stress = s_ij + de_ij tr(stress) / 3 = q / q_trial * s_ij^trial + de_ij p / 3 = q / q_trial *
  // (stress_ij^trial - de_ij tr(stress^trial) / 3) + de_ij p / 3
  const Real p_trial = stress_trial.trace();
  stress = RankTwoTensor(RankTwoTensor::initIdentity) / 3.0 *
           (p_ok - (_in_q_trial == 0.0 ? 0.0 : p_trial * q_ok / _in_q_trial));
  if (_in_q_trial > 0)
    stress += q_ok / _in_q_trial * stress_trial;
}

RankTwoTensor
CappedDruckerPragerStressUpdate::dpdstress(const RankTwoTensor & stress) const
{
  return stress.dtrace();
}

RankFourTensor
CappedDruckerPragerStressUpdate::d2pdstress2(const RankTwoTensor & /*stress*/) const
{
  return RankFourTensor();
}

RankTwoTensor
CappedDruckerPragerStressUpdate::dqdstress(const RankTwoTensor & stress) const
{
  const Real j2 = stress.secondInvariant();
  if (j2 == 0.0)
    return RankTwoTensor();
  return 0.5 * stress.dsecondInvariant() / std::sqrt(j2);
}

RankFourTensor
CappedDruckerPragerStressUpdate::d2qdstress2(const RankTwoTensor & stress) const
{
  const Real j2 = stress.secondInvariant();
  if (j2 == 0.0)
    return RankFourTensor();

  const RankTwoTensor dj2 = stress.dsecondInvariant();
  return -0.25 * dj2.outerProduct(dj2) / std::pow(j2, 1.5) +
         0.5 * stress.d2secondInvariant() / std::sqrt(j2);
}

void
CappedDruckerPragerStressUpdate::consistentTangentOperator(const RankTwoTensor & /*stress_trial*/,
                                                           Real /*p_trial*/,
                                                           Real /*q_trial*/,
                                                           const RankTwoTensor & stress,
                                                           Real /*p*/,
                                                           Real q,
                                                           Real gaE,
                                                           const yieldAndFlow & smoothed_q,
                                                           const RankFourTensor & Eijkl,
                                                           bool compute_full_tangent_operator,
                                                           RankFourTensor & cto) const
{
  cto = Eijkl;
  if (!compute_full_tangent_operator)
    return;

  const RankTwoTensor s_over_q =
      (q == 0.0 ? RankTwoTensor()
                : (stress - stress.trace() * RankTwoTensor(RankTwoTensor::initIdentity) / 3.0) / q);

  for (unsigned i = 0; i < _tensor_dimensionality; ++i)
    for (unsigned j = 0; j < _tensor_dimensionality; ++j)
      for (unsigned k = 0; k < _tensor_dimensionality; ++k)
        for (unsigned l = 0; l < _tensor_dimensionality; ++l)
        {
          cto(i, j, k, l) -=
              (i == j) * (1.0 / 3.0) *
              (_Epp * (1.0 - _dp_dpt) / 3.0 * (k == l) + s_over_q(k, l) * _Eqq * (-_dp_dqt));
          cto(i, j, k, l) -= s_over_q(i, j) * (_Epp * (-_dq_dpt) / 3.0 * (k == l) +
                                               s_over_q(k, l) * _Eqq * (1.0 - _dq_dqt));
        }

  if (smoothed_q.dg[1] != 0.0)
  {
    const RankFourTensor Tijab = Eijkl * (gaE / _Epp) * smoothed_q.dg[1] * d2qdstress2(stress);
    RankFourTensor inv = RankFourTensor(RankFourTensor::initIdentityFour) + Tijab;
    try
    {
      inv = inv.transposeMajor().invSymm();
    }
    catch (const MooseException & e)
    {
      // Cannot form the inverse, so probably at some degenerate place in stress space.
      // Just return with the "best estimate" of the cto.
      mooseWarning("CappedDruckerPragerStressUpdate: Cannot invert 1+T in consistent tangent "
                   "operator computation at quadpoint ",
                   _qp,
                   " of element ",
                   _current_elem->id());
      return;
    }
    cto = (cto.transposeMajor() * inv).transposeMajor();
  }
}
