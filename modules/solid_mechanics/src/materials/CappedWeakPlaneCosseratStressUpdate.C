//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CappedWeakPlaneCosseratStressUpdate.h"

#include "libmesh/utility.h"

registerMooseObject("TensorMechanicsApp", CappedWeakPlaneCosseratStressUpdate);

InputParameters
CappedWeakPlaneCosseratStressUpdate::validParams()
{
  InputParameters params = CappedWeakPlaneStressUpdate::validParams();
  params.addClassDescription("Capped weak-plane plasticity Cosserat stress calculator");
  return params;
}

CappedWeakPlaneCosseratStressUpdate::CappedWeakPlaneCosseratStressUpdate(
    const InputParameters & parameters)
  : CappedWeakPlaneStressUpdate(parameters)
{
}

void
CappedWeakPlaneCosseratStressUpdate::setStressAfterReturn(const RankTwoTensor & stress_trial,
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
    stress(0, 2) = stress(1, 2) = 0.0;
  else
  {
    stress(0, 2) = _in_trial02 * q_ok / _in_q_trial;
    stress(1, 2) = _in_trial12 * q_ok / _in_q_trial;
  }
}

void
CappedWeakPlaneCosseratStressUpdate::consistentTangentOperator(
    const RankTwoTensor & /*stress_trial*/,
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
  const Real Exzxz = Eijkl(0, 2, 0, 2);
  const Real tanpsi = _tan_psi.value(_intnl[_qp][0]);
  const Real dintnl0_dq = -1.0 / Exzxz;
  const Real dintnl0_dqt = 1.0 / Exzxz;
  const Real dintnl1_dp = -1.0 / Ezzzz;
  const Real dintnl1_dpt = 1.0 / Ezzzz;
  const Real dintnl1_dq =
      tanpsi / Exzxz - (q_trial - q) * _tan_psi.derivative(_intnl[_qp][0]) * dintnl0_dq / Exzxz;
  const Real dintnl1_dqt =
      -tanpsi / Exzxz - (q_trial - q) * _tan_psi.derivative(_intnl[_qp][0]) * dintnl0_dqt / Exzxz;

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
      cto(0, 2, i, i) = _in_trial02 / q_trial * _dq_dpt * dpt_depii;
      cto(1, 2, i, i) = _in_trial12 / q_trial * _dq_dpt * dpt_depii;
    }
  }

  const Real poisson_effect =
      -Eijkl(2, 2, 0, 0) / Ezzzz *
      (_dgaE_dqt * smoothed_q.dg[0] + gaE * smoothed_q.d2g[0][0] * _dp_dqt +
       gaE * smoothed_q.d2g[0][1] * _dq_dqt +
       gaE * smoothed_q.d2g_di[0][0] * (dintnl0_dqt + dintnl0_dq * _dq_dqt) +
       gaE * smoothed_q.d2g_di[0][1] * (dintnl1_dqt + dintnl1_dp * _dp_dqt + dintnl1_dq * _dq_dqt));

  const Real dqt_dep02 = (q_trial == 0.0 ? 1.0 : _in_trial02 / q_trial) * Eijkl(0, 2, 0, 2);
  cto(2, 2, 0, 2) = _dp_dqt * dqt_dep02;
  cto(0, 0, 0, 2) = cto(1, 1, 0, 2) = poisson_effect * dqt_dep02;
  if (q_trial > 0.0)
  {
    // for q_trial=0, Jacobian_mult is just given by the elastic case
    cto(0, 2, 0, 2) = Eijkl(0, 2, 0, 2) * q / q_trial +
                      _in_trial02 * (_dq_dqt - q / q_trial) / q_trial * dqt_dep02;
    cto(1, 2, 0, 2) = _in_trial12 * (_dq_dqt - q / q_trial) / q_trial * dqt_dep02;
  }

  const Real dqt_dep20 = (q_trial == 0.0 ? 1.0 : _in_trial02 / q_trial) * Eijkl(0, 2, 2, 0);
  cto(2, 2, 2, 0) = _dp_dqt * dqt_dep20;
  cto(0, 0, 2, 0) = cto(1, 1, 2, 0) = poisson_effect * dqt_dep20;
  if (q_trial > 0.0)
  {
    // for q_trial=0, Jacobian_mult is just given by the elastic case
    cto(0, 2, 2, 0) = Eijkl(0, 2, 2, 0) * q / q_trial +
                      _in_trial02 * (_dq_dqt - q / q_trial) / q_trial * dqt_dep20;
    cto(1, 2, 2, 0) = _in_trial12 * (_dq_dqt - q / q_trial) / q_trial * dqt_dep20;
  }

  const Real dqt_dep12 = (q_trial == 0.0 ? 1.0 : _in_trial12 / q_trial) * Eijkl(1, 2, 1, 2);
  cto(2, 2, 1, 2) = _dp_dqt * dqt_dep12;
  cto(0, 0, 1, 2) = cto(1, 1, 1, 2) = poisson_effect * dqt_dep12;
  if (q_trial > 0.0)
  {
    // for q_trial=0, Jacobian_mult is just given by the elastic case
    cto(0, 2, 1, 2) = _in_trial02 * (_dq_dqt - q / q_trial) / q_trial * dqt_dep12;
    cto(1, 2, 1, 2) = Eijkl(1, 2, 1, 2) * q / q_trial +
                      _in_trial12 * (_dq_dqt - q / q_trial) / q_trial * dqt_dep12;
  }

  const Real dqt_dep21 = (q_trial == 0.0 ? 1.0 : _in_trial12 / q_trial) * Eijkl(1, 2, 2, 1);
  cto(2, 2, 2, 1) = _dp_dqt * dqt_dep21;
  cto(0, 0, 2, 1) = cto(1, 1, 2, 1) = poisson_effect * dqt_dep21;
  if (q_trial > 0.0)
  {
    // for q_trial=0, Jacobian_mult is just given by the elastic case
    cto(0, 2, 2, 1) = _in_trial02 * (_dq_dqt - q / q_trial) / q_trial * dqt_dep21;
    cto(1, 2, 2, 1) = Eijkl(1, 2, 2, 1) * q / q_trial +
                      _in_trial12 * (_dq_dqt - q / q_trial) / q_trial * dqt_dep21;
  }
}

RankTwoTensor
CappedWeakPlaneCosseratStressUpdate::dqdstress(const RankTwoTensor & stress) const
{
  RankTwoTensor deriv = RankTwoTensor();
  const Real q = std::sqrt(Utility::pow<2>(stress(0, 2)) + Utility::pow<2>(stress(1, 2)));
  if (q > 0.0)
  {
    deriv(0, 2) = stress(0, 2) / q;
    deriv(1, 2) = stress(1, 2) / q;
  }
  else
  {
    // derivative is not defined here.  For now i'll set:
    deriv(0, 2) = 1.0;
    deriv(1, 2) = 1.0;
  }
  return deriv;
}

RankFourTensor
CappedWeakPlaneCosseratStressUpdate::d2qdstress2(const RankTwoTensor & stress) const
{
  RankFourTensor d2 = RankFourTensor();

  const Real q = std::sqrt(Utility::pow<2>(stress(0, 2)) + Utility::pow<2>(stress(1, 2)));
  if (q == 0.0)
    return d2;

  const Real dq02 = stress(0, 2) / q;
  const Real dq12 = stress(1, 2) / q;

  d2(0, 2, 0, 2) = 1.0 / q - dq02 * dq02 / q;
  d2(0, 2, 1, 2) = -dq02 * dq12 / q;
  d2(1, 2, 0, 2) = -dq12 * dq02 / q;
  d2(1, 2, 1, 2) = 1.0 / q - dq12 * dq12 / q;

  return d2;
}
