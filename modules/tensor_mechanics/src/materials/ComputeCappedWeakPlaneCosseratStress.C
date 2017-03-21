/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ComputeCappedWeakPlaneCosseratStress.h"

#include "libmesh/utility.h"

template <>
InputParameters
validParams<ComputeCappedWeakPlaneCosseratStress>()
{
  InputParameters params = validParams<ComputeCappedWeakPlaneStress>();
  params.addClassDescription("Capped weak-plane plasticity Cosserat stress calculator");
  return params;
}

ComputeCappedWeakPlaneCosseratStress::ComputeCappedWeakPlaneCosseratStress(
    const InputParameters & parameters)
  : ComputeCappedWeakPlaneStress(parameters),
    _curvature(getMaterialPropertyByName<RankTwoTensor>("curvature")),
    _elastic_flexural_rigidity_tensor(
        getMaterialPropertyByName<RankFourTensor>("elastic_flexural_rigidity_tensor")),
    _couple_stress(declareProperty<RankTwoTensor>("couple_stress")),
    _couple_stress_old(declarePropertyOld<RankTwoTensor>("couple_stress")),
    _Jacobian_mult_couple(declareProperty<RankFourTensor>("couple_Jacobian_mult"))
{
}

void
ComputeCappedWeakPlaneCosseratStress::initQpStatefulProperties()
{
  ComputeCappedWeakPlaneStress::initQpStatefulProperties();
  _couple_stress[_qp].zero();
}

void
ComputeCappedWeakPlaneCosseratStress::initialiseReturnProcess()
{
  ComputeCappedWeakPlaneStress::initialiseReturnProcess();
  _couple_stress[_qp] = _elastic_flexural_rigidity_tensor[_qp] * _curvature[_qp];
  if (_fe_problem.currentlyComputingJacobian())
    _Jacobian_mult_couple[_qp] = _elastic_flexural_rigidity_tensor[_qp];
}

void
ComputeCappedWeakPlaneCosseratStress::setStressAfterReturn(const RankTwoTensor & stress_trial,
                                                           Real p_ok,
                                                           Real q_ok,
                                                           Real gaE,
                                                           const std::vector<Real> & /*intnl*/,
                                                           const f_and_derivs & smoothed_q,
                                                           RankTwoTensor & stress) const
{
  stress = stress_trial;
  stress(2, 2) = p_ok;
  // stress_xx and stress_yy are sitting at their trial-stress values
  // so need to bring them back via Poisson's ratio
  stress(0, 0) -= _elasticity_tensor[_qp](2, 2, 0, 0) * gaE / _Epp * smoothed_q.dg[0];
  stress(1, 1) -= _elasticity_tensor[_qp](2, 2, 1, 1) * gaE / _Epp * smoothed_q.dg[0];
  if (_in_q_trial == 0.0)
    stress(0, 2) = stress(1, 2) = 0.0;
  else
  {
    stress(0, 2) = _in_trial02 * q_ok / _in_q_trial;
    stress(1, 2) = _in_trial12 * q_ok / _in_q_trial;
  }
}

void
ComputeCappedWeakPlaneCosseratStress::consistentTangentOperator(
    const RankTwoTensor & /*stress_trial*/,
    Real /*p_trial*/,
    Real q_trial,
    const RankTwoTensor & /*stress*/,
    Real /*p*/,
    Real q,
    Real gaE,
    const f_and_derivs & smoothed_q,
    RankFourTensor & cto) const
{
  if (!_fe_problem.currentlyComputingJacobian())
    return;

  cto = _elasticity_tensor[_qp];
  if (_tangent_operator_type == TangentOperatorEnum::elastic)
    return;

  const Real Ezzzz = _elasticity_tensor[_qp](2, 2, 2, 2);
  const Real Exzxz = _elasticity_tensor[_qp](0, 2, 0, 2);
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
    const Real dpt_depii = _elasticity_tensor[_qp](2, 2, i, i);
    cto(2, 2, i, i) = _dp_dpt * dpt_depii;
    const Real poisson_effect =
        _elasticity_tensor[_qp](2, 2, 0, 0) / Ezzzz *
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
      -_elasticity_tensor[_qp](2, 2, 0, 0) / Ezzzz *
      (_dgaE_dqt * smoothed_q.dg[0] + gaE * smoothed_q.d2g[0][0] * _dp_dqt +
       gaE * smoothed_q.d2g[0][1] * _dq_dqt +
       gaE * smoothed_q.d2g_di[0][0] * (dintnl0_dqt + dintnl0_dq * _dq_dqt) +
       gaE * smoothed_q.d2g_di[0][1] * (dintnl1_dqt + dintnl1_dp * _dp_dqt + dintnl1_dq * _dq_dqt));

  const Real dqt_dep02 =
      (q_trial == 0.0 ? 1.0 : _in_trial02 / q_trial) * _elasticity_tensor[_qp](0, 2, 0, 2);
  cto(2, 2, 0, 2) = _dp_dqt * dqt_dep02;
  cto(0, 0, 0, 2) = cto(1, 1, 0, 2) = poisson_effect * dqt_dep02;
  if (q_trial > 0.0)
  {
    // for q_trial=0, Jacobian_mult is just given by the elastic case
    cto(0, 2, 0, 2) = _elasticity_tensor[_qp](0, 2, 0, 2) * q / q_trial +
                      _in_trial02 * (_dq_dqt - q / q_trial) / q_trial * dqt_dep02;
    cto(1, 2, 0, 2) = _in_trial12 * (_dq_dqt - q / q_trial) / q_trial * dqt_dep02;
  }

  const Real dqt_dep20 =
      (q_trial == 0.0 ? 1.0 : _in_trial02 / q_trial) * _elasticity_tensor[_qp](0, 2, 2, 0);
  cto(2, 2, 2, 0) = _dp_dqt * dqt_dep20;
  cto(0, 0, 2, 0) = cto(1, 1, 2, 0) = poisson_effect * dqt_dep20;
  if (q_trial > 0.0)
  {
    // for q_trial=0, Jacobian_mult is just given by the elastic case
    cto(0, 2, 2, 0) = _elasticity_tensor[_qp](0, 2, 2, 0) * q / q_trial +
                      _in_trial02 * (_dq_dqt - q / q_trial) / q_trial * dqt_dep20;
    cto(1, 2, 2, 0) = _in_trial12 * (_dq_dqt - q / q_trial) / q_trial * dqt_dep20;
  }

  const Real dqt_dep12 =
      (q_trial == 0.0 ? 1.0 : _in_trial12 / q_trial) * _elasticity_tensor[_qp](1, 2, 1, 2);
  cto(2, 2, 1, 2) = _dp_dqt * dqt_dep12;
  cto(0, 0, 1, 2) = cto(1, 1, 1, 2) = poisson_effect * dqt_dep12;
  if (q_trial > 0.0)
  {
    // for q_trial=0, Jacobian_mult is just given by the elastic case
    cto(0, 2, 1, 2) = _in_trial02 * (_dq_dqt - q / q_trial) / q_trial * dqt_dep12;
    cto(1, 2, 1, 2) = _elasticity_tensor[_qp](1, 2, 1, 2) * q / q_trial +
                      _in_trial12 * (_dq_dqt - q / q_trial) / q_trial * dqt_dep12;
  }

  const Real dqt_dep21 =
      (q_trial == 0.0 ? 1.0 : _in_trial12 / q_trial) * _elasticity_tensor[_qp](1, 2, 2, 1);
  cto(2, 2, 2, 1) = _dp_dqt * dqt_dep21;
  cto(0, 0, 2, 1) = cto(1, 1, 2, 1) = poisson_effect * dqt_dep21;
  if (q_trial > 0.0)
  {
    // for q_trial=0, Jacobian_mult is just given by the elastic case
    cto(0, 2, 2, 1) = _in_trial02 * (_dq_dqt - q / q_trial) / q_trial * dqt_dep21;
    cto(1, 2, 2, 1) = _elasticity_tensor[_qp](1, 2, 2, 1) * q / q_trial +
                      _in_trial12 * (_dq_dqt - q / q_trial) / q_trial * dqt_dep21;
  }
}

RankTwoTensor
ComputeCappedWeakPlaneCosseratStress::dqdstress(const RankTwoTensor & stress) const
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
ComputeCappedWeakPlaneCosseratStress::d2qdstress2(const RankTwoTensor & stress) const
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
