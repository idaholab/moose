//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CappedWeakPlaneStressUpdate.h"

#include "libmesh/utility.h"

registerMooseObject("TensorMechanicsApp", CappedWeakPlaneStressUpdate);

InputParameters
CappedWeakPlaneStressUpdate::validParams()
{
  InputParameters params = TwoParameterPlasticityStressUpdate::validParams();
  params.addClassDescription("Capped weak-plane plasticity stress calculator");
  params.addRequiredParam<UserObjectName>(
      "cohesion",
      "A TensorMechanicsHardening UserObject that defines hardening of the cohesion.  "
      "Physically the cohesion should not be negative.");
  params.addRequiredParam<UserObjectName>("tan_friction_angle",
                                          "A TensorMechanicsHardening UserObject that defines "
                                          "hardening of tan(friction angle).  Physically the "
                                          "friction angle should be between 0 and 90deg.");
  params.addRequiredParam<UserObjectName>(
      "tan_dilation_angle",
      "A TensorMechanicsHardening UserObject that defines hardening of the "
      "tan(dilation angle).  Usually the dilation angle is not greater than "
      "the friction angle, and it is between 0 and 90deg.");
  params.addRequiredParam<UserObjectName>(
      "tensile_strength",
      "A TensorMechanicsHardening UserObject that defines hardening of the "
      "weak-plane tensile strength.  In physical situations this is positive "
      "(and always must be greater than negative compressive-strength.");
  params.addRequiredParam<UserObjectName>("compressive_strength",
                                          "A TensorMechanicsHardening UserObject that defines "
                                          "hardening of the weak-plane compressive strength.  In "
                                          "physical situations this is positive.");
  params.addRequiredRangeCheckedParam<Real>(
      "tip_smoother",
      "tip_smoother>=0",
      "The cone vertex at shear-stress = 0 will be smoothed by "
      "the given amount.  Typical value is 0.1*cohesion");
  params.addParam<bool>("perfect_guess",
                        true,
                        "Provide a guess to the Newton-Raphson procedure "
                        "that is the result from perfect plasticity.  With "
                        "severe hardening/softening this may be "
                        "suboptimal.");
  return params;
}

CappedWeakPlaneStressUpdate::CappedWeakPlaneStressUpdate(const InputParameters & parameters)
  : TwoParameterPlasticityStressUpdate(parameters, 3, 2),
    _cohesion(getUserObject<TensorMechanicsHardeningModel>("cohesion")),
    _tan_phi(getUserObject<TensorMechanicsHardeningModel>("tan_friction_angle")),
    _tan_psi(getUserObject<TensorMechanicsHardeningModel>("tan_dilation_angle")),
    _tstrength(getUserObject<TensorMechanicsHardeningModel>("tensile_strength")),
    _cstrength(getUserObject<TensorMechanicsHardeningModel>("compressive_strength")),
    _small_smoother2(Utility::pow<2>(getParam<Real>("tip_smoother"))),
    _perfect_guess(getParam<bool>("perfect_guess")),
    _stress_return_type(StressReturnType::nothing_special),
    _in_trial02(0.0),
    _in_trial12(0.0),
    _in_q_trial(0.0)
{
  // With arbitary UserObjects, it is impossible to check everything,
  // but this will catch the common errors
  if (_tan_phi.value(0) < 0 || _tan_psi.value(0) < 0)
    mooseError("CappedWeakPlaneStressUpdate: Weak-plane friction and dilation angles must lie in "
               "[0, Pi/2]");
  if (_tan_phi.value(0) < _tan_psi.value(0))
    mooseError("CappedWeakPlaneStressUpdate: Weak-plane friction angle must not be less than "
               "dilation angle");
  if (_cohesion.value(0) < 0)
    mooseError("CappedWeakPlaneStressUpdate: Weak-plane cohesion must not be negative");
  if (_tstrength.value(0) + _cstrength.value(0) <= _smoothing_tol)
    mooseError("CappedWeakPlaneStressUpdate: Weak plane tensile strength plus compressive "
               "strength must be greater than smoothing_tol");
}

void
CappedWeakPlaneStressUpdate::initializeReturnProcess()
{
  _stress_return_type = StressReturnType::nothing_special;
}

void
CappedWeakPlaneStressUpdate::finalizeReturnProcess(const RankTwoTensor & /*rotation_increment*/)
{
  _stress_return_type = StressReturnType::nothing_special;
}

void
CappedWeakPlaneStressUpdate::preReturnMap(Real /*p_trial*/,
                                          Real q_trial,
                                          const RankTwoTensor & stress_trial,
                                          const std::vector<Real> & /*intnl_old*/,
                                          const std::vector<Real> & yf,
                                          const RankFourTensor & /*Eijkl*/)
{
  // If it's obvious, then simplify the return-type
  if (yf[1] >= 0)
    _stress_return_type = StressReturnType::no_compression;
  else if (yf[2] >= 0)
    _stress_return_type = StressReturnType::no_tension;

  // The following are useful for the Cosserat case too
  _in_trial02 = stress_trial(0, 2);
  _in_trial12 = stress_trial(1, 2);
  _in_q_trial = q_trial;
}

void
CappedWeakPlaneStressUpdate::computePQ(const RankTwoTensor & stress, Real & p, Real & q) const
{
  p = stress(2, 2);
  // Because the following is not explicitly symmeterised, it is useful for the Cosserat case too
  q = std::sqrt(Utility::pow<2>(stress(0, 2)) + Utility::pow<2>(stress(1, 2)));
}

void
CappedWeakPlaneStressUpdate::setEppEqq(const RankFourTensor & Eijkl, Real & Epp, Real & Eqq) const
{
  Epp = Eijkl(2, 2, 2, 2);
  Eqq = Eijkl(0, 2, 0, 2);
}

void
CappedWeakPlaneStressUpdate::setStressAfterReturn(const RankTwoTensor & stress_trial,
                                                  Real p_ok,
                                                  Real q_ok,
                                                  Real gaE,
                                                  const std::vector<Real> & /*intnl*/,
                                                  const yieldAndFlow & smoothed_q,
                                                  const RankFourTensor & Eijkl,
                                                  RankTwoTensor & stress) const
{
  stress = stress_trial;
  stress(2, 2) = p_ok;
  // stress_xx and stress_yy are sitting at their trial-stress values
  // so need to bring them back via Poisson's ratio
  stress(0, 0) -= Eijkl(2, 2, 0, 0) * gaE / _Epp * smoothed_q.dg[0];
  stress(1, 1) -= Eijkl(2, 2, 1, 1) * gaE / _Epp * smoothed_q.dg[0];
  if (_in_q_trial == 0.0)
    stress(2, 0) = stress(2, 1) = stress(0, 2) = stress(1, 2) = 0.0;
  else
  {
    stress(2, 0) = stress(0, 2) = _in_trial02 * q_ok / _in_q_trial;
    stress(2, 1) = stress(1, 2) = _in_trial12 * q_ok / _in_q_trial;
  }
}

void
CappedWeakPlaneStressUpdate::setIntnlDerivatives(Real /*p_trial*/,
                                                 Real q_trial,
                                                 Real /*p*/,
                                                 Real q,
                                                 const std::vector<Real> & intnl,
                                                 std::vector<std::vector<Real>> & dintnl) const
{
  const Real tanpsi = _tan_psi.value(intnl[0]);
  dintnl[0][0] = 0.0;
  dintnl[0][1] = -1.0 / _Eqq;
  dintnl[1][0] = -1.0 / _Epp;
  dintnl[1][1] =
      tanpsi / _Eqq - (q_trial - q) * _tan_psi.derivative(intnl[0]) * dintnl[0][1] / _Eqq;
}

void
CappedWeakPlaneStressUpdate::consistentTangentOperator(const RankTwoTensor & /*stress_trial*/,
                                                       Real /*p_trial*/,
                                                       Real q_trial,
                                                       const RankTwoTensor & /*stress*/,
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

  const Real Ezzzz = Eijkl(2, 2, 2, 2);
  const Real Ezxzx = Eijkl(2, 0, 2, 0);
  const Real tanpsi = _tan_psi.value(_intnl[_qp][0]);
  const Real dintnl0_dq = -1.0 / Ezxzx;
  const Real dintnl0_dqt = 1.0 / Ezxzx;
  const Real dintnl1_dp = -1.0 / Ezzzz;
  const Real dintnl1_dpt = 1.0 / Ezzzz;
  const Real dintnl1_dq =
      tanpsi / Ezxzx - (q_trial - q) * _tan_psi.derivative(_intnl[_qp][0]) * dintnl0_dq / Ezxzx;
  const Real dintnl1_dqt =
      -tanpsi / Ezxzx - (q_trial - q) * _tan_psi.derivative(_intnl[_qp][0]) * dintnl0_dqt / Ezxzx;

  for (unsigned i = 0; i < _tensor_dimensionality; ++i)
  {
    const Real dpt_depii = Eijkl(2, 2, i, i);
    cto(2, 2, i, i) = _dp_dpt * dpt_depii;
    const Real poisson_effect =
        Eijkl(2, 2, 0, 0) / Ezzzz *
        (_dgaE_dpt * smoothed_q.dg[0] + gaE * smoothed_q.d2g[0][0] * _dp_dpt +
         gaE * smoothed_q.d2g[0][1] * _dq_dpt +
         gaE * smoothed_q.d2g_di[0][0] * (dintnl0_dq * _dq_dpt) +
         gaE * smoothed_q.d2g_di[0][1] *
             (dintnl1_dpt + dintnl1_dp * _dp_dpt + dintnl1_dq * _dq_dpt)) *
        dpt_depii;
    cto(0, 0, i, i) -= poisson_effect;
    cto(1, 1, i, i) -= poisson_effect;
    if (q_trial > 0.0)
    {
      cto(2, 0, i, i) = cto(0, 2, i, i) = _in_trial02 / q_trial * _dq_dpt * dpt_depii;
      cto(2, 1, i, i) = cto(1, 2, i, i) = _in_trial12 / q_trial * _dq_dpt * dpt_depii;
    }
  }

  const Real poisson_effect =
      -Eijkl(2, 2, 0, 0) / Ezzzz *
      (_dgaE_dqt * smoothed_q.dg[0] + gaE * smoothed_q.d2g[0][0] * _dp_dqt +
       gaE * smoothed_q.d2g[0][1] * _dq_dqt +
       gaE * smoothed_q.d2g_di[0][0] * (dintnl0_dqt + dintnl0_dq * _dq_dqt) +
       gaE * smoothed_q.d2g_di[0][1] * (dintnl1_dqt + dintnl1_dp * _dp_dqt + dintnl1_dq * _dq_dqt));

  const Real dqt_dep20 = (q_trial == 0.0 ? 1.0 : _in_trial02 / q_trial) * Eijkl(2, 0, 2, 0);
  cto(2, 2, 2, 0) = cto(2, 2, 0, 2) = _dp_dqt * dqt_dep20;
  cto(0, 0, 2, 0) = cto(0, 0, 0, 2) = cto(1, 1, 2, 0) = cto(1, 1, 0, 2) =
      poisson_effect * dqt_dep20;
  if (q_trial > 0.0)
  {
    // for q_trial=0, Jacobian_mult is just given by the elastic case
    cto(0, 2, 0, 2) = cto(2, 0, 0, 2) = cto(0, 2, 2, 0) = cto(2, 0, 2, 0) =
        Eijkl(2, 0, 2, 0) * q / q_trial +
        _in_trial02 * (_dq_dqt - q / q_trial) / q_trial * dqt_dep20;
    cto(1, 2, 0, 2) = cto(2, 1, 0, 2) = cto(1, 2, 2, 0) = cto(2, 1, 2, 0) =
        _in_trial12 * (_dq_dqt - q / q_trial) / q_trial * dqt_dep20;
  }

  const Real dqt_dep21 = (q_trial == 0.0 ? 1.0 : _in_trial12 / q_trial) * Eijkl(2, 1, 2, 1);
  cto(2, 2, 2, 1) = cto(2, 2, 1, 2) = _dp_dqt * dqt_dep21;
  cto(0, 0, 2, 1) = cto(0, 0, 1, 2) = cto(1, 1, 2, 1) = cto(1, 1, 1, 2) =
      poisson_effect * dqt_dep21;
  if (q_trial > 0.0)
  {
    // for q_trial=0, Jacobian_mult is just given by the elastic case
    cto(0, 2, 1, 2) = cto(2, 0, 1, 2) = cto(0, 2, 2, 1) = cto(2, 0, 2, 1) =
        _in_trial02 * (_dq_dqt - q / q_trial) / q_trial * dqt_dep21;
    cto(1, 2, 1, 2) = cto(2, 1, 1, 2) = cto(1, 2, 2, 1) = cto(2, 1, 2, 1) =
        Eijkl(2, 1, 2, 1) * q / q_trial +
        _in_trial12 * (_dq_dqt - q / q_trial) / q_trial * dqt_dep21;
  }
}

void
CappedWeakPlaneStressUpdate::yieldFunctionValues(Real p,
                                                 Real q,
                                                 const std::vector<Real> & intnl,
                                                 std::vector<Real> & yf) const
{
  yf[0] = std::sqrt(Utility::pow<2>(q) + _small_smoother2) + p * _tan_phi.value(intnl[0]) -
          _cohesion.value(intnl[0]);

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
CappedWeakPlaneStressUpdate::computeAllQ(Real p,
                                         Real q,
                                         const std::vector<Real> & intnl,
                                         std::vector<yieldAndFlow> & all_q) const
{
  // yield function values
  all_q[0].f = std::sqrt(Utility::pow<2>(q) + _small_smoother2) + p * _tan_phi.value(intnl[0]) -
               _cohesion.value(intnl[0]);
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
  all_q[0].df[0] = _tan_phi.value(intnl[0]);
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
  all_q[0].df_di[0] = p * _tan_phi.derivative(intnl[0]) - _cohesion.derivative(intnl[0]);
  all_q[1].df_di[0] = 0.0;
  all_q[2].df_di[0] = 0.0;
  // derivatives wrt intnl[q] (tensile plastic strain)
  all_q[0].df_di[1] = 0.0;
  all_q[1].df_di[1] = -_tstrength.derivative(intnl[1]);
  all_q[2].df_di[1] = -_cstrength.derivative(intnl[1]);

  // d(flowPotential)/d(p, q)
  // derivatives wrt p
  all_q[0].dg[0] = _tan_psi.value(intnl[0]);
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
  all_q[0].d2g_di[0][0] = _tan_psi.derivative(intnl[0]);
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
CappedWeakPlaneStressUpdate::initializeVars(Real p_trial,
                                            Real q_trial,
                                            const std::vector<Real> & intnl_old,
                                            Real & p,
                                            Real & q,
                                            Real & gaE,
                                            std::vector<Real> & intnl) const
{
  const Real tanpsi = _tan_psi.value(intnl_old[0]);

  if (!_perfect_guess)
  {
    p = p_trial;
    q = q_trial;
    gaE = 0.0;
  }
  else
  {
    const Real coh = _cohesion.value(intnl_old[0]);
    const Real tanphi = _tan_phi.value(intnl_old[0]);
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
CappedWeakPlaneStressUpdate::setIntnlValues(Real p_trial,
                                            Real q_trial,
                                            Real p,
                                            Real q,
                                            const std::vector<Real> & intnl_old,
                                            std::vector<Real> & intnl) const
{
  intnl[0] = intnl_old[0] + (q_trial - q) / _Eqq;
  const Real tanpsi = _tan_psi.value(intnl[0]);
  intnl[1] = intnl_old[1] + (p_trial - p) / _Epp - (q_trial - q) * tanpsi / _Eqq;
}

RankTwoTensor
CappedWeakPlaneStressUpdate::dpdstress(const RankTwoTensor & /*stress*/) const
{
  RankTwoTensor deriv = RankTwoTensor();
  deriv(2, 2) = 1.0;
  return deriv;
}

RankFourTensor
CappedWeakPlaneStressUpdate::d2pdstress2(const RankTwoTensor & /*stress*/) const
{
  return RankFourTensor();
}

RankTwoTensor
CappedWeakPlaneStressUpdate::dqdstress(const RankTwoTensor & stress) const
{
  RankTwoTensor deriv = RankTwoTensor();
  const Real q = std::sqrt(Utility::pow<2>(stress(2, 0)) + Utility::pow<2>(stress(2, 1)));
  if (q > 0.0)
  {
    deriv(2, 0) = deriv(0, 2) = 0.5 * stress(2, 0) / q;
    deriv(2, 1) = deriv(1, 2) = 0.5 * stress(2, 1) / q;
  }
  else
  {
    // derivative is not defined here.  For now i'll set:
    deriv(2, 0) = deriv(0, 2) = 0.5;
    deriv(2, 1) = deriv(1, 2) = 0.5;
  }
  return deriv;
}

RankFourTensor
CappedWeakPlaneStressUpdate::d2qdstress2(const RankTwoTensor & stress) const
{
  RankFourTensor d2 = RankFourTensor();

  const Real q = std::sqrt(Utility::pow<2>(stress(2, 0)) + Utility::pow<2>(stress(2, 1)));
  if (q == 0.0)
    return d2;

  RankTwoTensor dq = RankTwoTensor();
  dq(2, 0) = dq(0, 2) = 0.25 * (stress(2, 0) + stress(0, 2)) / q;
  dq(2, 1) = dq(1, 2) = 0.25 * (stress(2, 1) + stress(1, 2)) / q;

  for (unsigned i = 0; i < _tensor_dimensionality; ++i)
    for (unsigned j = 0; j < _tensor_dimensionality; ++j)
      for (unsigned k = 0; k < _tensor_dimensionality; ++k)
        for (unsigned l = 0; l < _tensor_dimensionality; ++l)
          d2(i, j, k, l) = -dq(i, j) * dq(k, l) / q;

  d2(0, 2, 0, 2) += 0.25 / q;
  d2(0, 2, 2, 0) += 0.25 / q;
  d2(2, 0, 0, 2) += 0.25 / q;
  d2(2, 0, 2, 0) += 0.25 / q;
  d2(1, 2, 1, 2) += 0.25 / q;
  d2(1, 2, 2, 1) += 0.25 / q;
  d2(2, 1, 1, 2) += 0.25 / q;
  d2(2, 1, 2, 1) += 0.25 / q;

  return d2;
}
