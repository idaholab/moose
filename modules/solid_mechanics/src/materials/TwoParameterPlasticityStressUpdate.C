//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TwoParameterPlasticityStressUpdate.h"

#include "Conversion.h"      // for stringify
#include "libmesh/utility.h" // for Utility::pow

InputParameters
TwoParameterPlasticityStressUpdate::validParams()
{
  InputParameters params = MultiParameterPlasticityStressUpdate::validParams();
  params.addClassDescription("Return-map and Jacobian algorithms for (P, Q) plastic models");
  return params;
}

TwoParameterPlasticityStressUpdate::TwoParameterPlasticityStressUpdate(
    const InputParameters & parameters, unsigned num_yf, unsigned num_intnl)
  : MultiParameterPlasticityStressUpdate(parameters, _num_pq, num_yf, num_intnl),
    _p_trial(0.0),
    _q_trial(0.0),
    _Epp(0.0),
    _Eqq(0.0),
    _dgaE_dpt(0.0),
    _dgaE_dqt(0.0),
    _dp_dpt(0.0),
    _dq_dpt(0.0),
    _dp_dqt(0.0),
    _dq_dqt(0.0)
{
}

void
TwoParameterPlasticityStressUpdate::yieldFunctionValuesV(const std::vector<Real> & stress_params,
                                                         const std::vector<Real> & intnl,
                                                         std::vector<Real> & yf) const
{
  const Real p = stress_params[0];
  const Real q = stress_params[1];
  yieldFunctionValues(p, q, intnl, yf);
}

void
TwoParameterPlasticityStressUpdate::setEffectiveElasticity(const RankFourTensor & Eijkl)
{
  setEppEqq(Eijkl, _Epp, _Eqq);
  _Eij[0][0] = _Epp;
  _Eij[1][0] = _Eij[0][1] = 0.0;
  _Eij[1][1] = _Eqq;
  _En = _Epp;
  _Cij[0][0] = 1.0 / _Epp;
  _Cij[1][0] = _Cij[0][1] = 0.0;
  _Cij[1][1] = 1.0 / _Eqq;
}

void
TwoParameterPlasticityStressUpdate::preReturnMapV(const std::vector<Real> & trial_stress_params,
                                                  const RankTwoTensor & stress_trial,
                                                  const std::vector<Real> & intnl_old,
                                                  const std::vector<Real> & yf,
                                                  const RankFourTensor & Eijkl)
{
  const Real p_trial = trial_stress_params[0];
  const Real q_trial = trial_stress_params[1];
  preReturnMap(p_trial, q_trial, stress_trial, intnl_old, yf, Eijkl);
}

void
TwoParameterPlasticityStressUpdate::preReturnMap(Real /*p_trial*/,
                                                 Real /*q_trial*/,
                                                 const RankTwoTensor & /*stress_trial*/,
                                                 const std::vector<Real> & /*intnl_old*/,
                                                 const std::vector<Real> & /*yf*/,
                                                 const RankFourTensor & /*Eijkl*/)
{
  return;
}

void
TwoParameterPlasticityStressUpdate::initializeVars(Real p_trial,
                                                   Real q_trial,
                                                   const std::vector<Real> & intnl_old,
                                                   Real & p,
                                                   Real & q,
                                                   Real & gaE,
                                                   std::vector<Real> & intnl) const
{
  p = p_trial;
  q = q_trial;
  gaE = 0.0;
  std::copy(intnl_old.begin(), intnl_old.end(), intnl.begin());
}

void
TwoParameterPlasticityStressUpdate::computeAllQV(const std::vector<Real> & stress_params,
                                                 const std::vector<Real> & intnl,
                                                 std::vector<yieldAndFlow> & all_q) const
{
  const Real p = stress_params[0];
  const Real q = stress_params[1];
  computeAllQ(p, q, intnl, all_q);
}

void
TwoParameterPlasticityStressUpdate::initializeVarsV(const std::vector<Real> & trial_stress_params,
                                                    const std::vector<Real> & intnl_old,
                                                    std::vector<Real> & stress_params,
                                                    Real & gaE,
                                                    std::vector<Real> & intnl) const
{
  const Real p_trial = trial_stress_params[0];
  const Real q_trial = trial_stress_params[1];
  Real p;
  Real q;
  initializeVars(p_trial, q_trial, intnl_old, p, q, gaE, intnl);
  stress_params[0] = p;
  stress_params[1] = q;
}

void
TwoParameterPlasticityStressUpdate::setIntnlValuesV(const std::vector<Real> & trial_stress_params,
                                                    const std::vector<Real> & current_stress_params,
                                                    const std::vector<Real> & intnl_old,
                                                    std::vector<Real> & intnl) const
{
  const Real p_trial = trial_stress_params[0];
  const Real q_trial = trial_stress_params[1];
  const Real p = current_stress_params[0];
  const Real q = current_stress_params[1];
  setIntnlValues(p_trial, q_trial, p, q, intnl_old, intnl);
}

void
TwoParameterPlasticityStressUpdate::setIntnlDerivativesV(
    const std::vector<Real> & trial_stress_params,
    const std::vector<Real> & current_stress_params,
    const std::vector<Real> & intnl_old,
    std::vector<std::vector<Real>> & dintnl) const
{
  const Real p_trial = trial_stress_params[0];
  const Real q_trial = trial_stress_params[1];
  const Real p = current_stress_params[0];
  const Real q = current_stress_params[1];
  setIntnlDerivatives(p_trial, q_trial, p, q, intnl_old, dintnl);
}

void
TwoParameterPlasticityStressUpdate::computeStressParams(const RankTwoTensor & stress,
                                                        std::vector<Real> & stress_params) const
{
  Real p;
  Real q;
  computePQ(stress, p, q);
  stress_params[0] = p;
  stress_params[1] = q;
}

void
TwoParameterPlasticityStressUpdate::consistentTangentOperatorV(
    const RankTwoTensor & stress_trial,
    const std::vector<Real> & trial_stress_params,
    const RankTwoTensor & stress,
    const std::vector<Real> & stress_params,
    Real gaE,
    const yieldAndFlow & smoothed_q,
    const RankFourTensor & Eijkl,
    bool compute_full_tangent_operator,
    const std::vector<std::vector<Real>> & dvar_dtrial,
    RankFourTensor & cto)
{
  const Real p_trial = trial_stress_params[0];
  const Real q_trial = trial_stress_params[1];
  const Real p = stress_params[0];
  const Real q = stress_params[1];
  _dp_dpt = dvar_dtrial[0][0];
  _dp_dqt = dvar_dtrial[0][1];
  _dq_dpt = dvar_dtrial[1][0];
  _dq_dqt = dvar_dtrial[1][1];
  _dgaE_dpt = dvar_dtrial[2][0];
  _dgaE_dqt = dvar_dtrial[2][1];
  consistentTangentOperator(stress_trial,
                            p_trial,
                            q_trial,
                            stress,
                            p,
                            q,
                            gaE,
                            smoothed_q,
                            Eijkl,
                            compute_full_tangent_operator,
                            cto);
}

void
TwoParameterPlasticityStressUpdate::consistentTangentOperator(
    const RankTwoTensor & stress_trial,
    Real /*p_trial*/,
    Real /*q_trial*/,
    const RankTwoTensor & stress,
    Real /*p*/,
    Real /*q*/,
    Real gaE,
    const yieldAndFlow & smoothed_q,
    const RankFourTensor & elasticity_tensor,
    bool compute_full_tangent_operator,
    RankFourTensor & cto) const
{
  cto = elasticity_tensor;
  if (!compute_full_tangent_operator)
    return;

  const RankTwoTensor dpdsig = dpdstress(stress);
  const RankTwoTensor dpdsig_trial = dpdstress(stress_trial);
  const RankTwoTensor dqdsig = dqdstress(stress);
  const RankTwoTensor dqdsig_trial = dqdstress(stress_trial);

  const RankTwoTensor s1 = elasticity_tensor * ((1.0 / _Epp) * (1.0 - _dp_dpt) * dpdsig +
                                                (1.0 / _Eqq) * (-_dq_dpt) * dqdsig);
  const RankTwoTensor s2 = elasticity_tensor * ((1.0 / _Epp) * (-_dp_dqt) * dpdsig +
                                                (1.0 / _Eqq) * (1.0 - _dq_dqt) * dqdsig);
  const RankTwoTensor t1 = elasticity_tensor * dpdsig_trial;
  const RankTwoTensor t2 = elasticity_tensor * dqdsig_trial;

  for (unsigned i = 0; i < _tensor_dimensionality; ++i)
    for (unsigned j = 0; j < _tensor_dimensionality; ++j)
      for (unsigned k = 0; k < _tensor_dimensionality; ++k)
        for (unsigned l = 0; l < _tensor_dimensionality; ++l)
          cto(i, j, k, l) -= s1(i, j) * t1(k, l) + s2(i, j) * t2(k, l);

  const RankFourTensor d2pdsig2 = d2pdstress2(stress);
  const RankFourTensor d2qdsig2 = d2qdstress2(stress);

  const RankFourTensor Tijab = elasticity_tensor * (gaE / _Epp) *
                               (smoothed_q.dg[0] * d2pdsig2 + smoothed_q.dg[1] * d2qdsig2);

  RankFourTensor inv = RankFourTensor(RankFourTensor::initIdentityFour) + Tijab;
  try
  {
    inv = inv.transposeMajor().invSymm();
  }
  catch (const MooseException & e)
  {
    // Cannot form the inverse, so probably at some degenerate place in stress space.
    // Just return with the "best estimate" of the cto.
    mooseWarning("TwoParameterPlasticityStressUpdate: Cannot invert 1+T in consistent tangent "
                 "operator computation at quadpoint ",
                 _qp,
                 " of element ",
                 _current_elem->id());
    return;
  }

  cto = (cto.transposeMajor() * inv).transposeMajor();
}

void
TwoParameterPlasticityStressUpdate::setStressAfterReturnV(const RankTwoTensor & stress_trial,
                                                          const std::vector<Real> & stress_params,
                                                          Real gaE,
                                                          const std::vector<Real> & intnl,
                                                          const yieldAndFlow & smoothed_q,
                                                          const RankFourTensor & Eijkl,
                                                          RankTwoTensor & stress) const
{
  const Real p_ok = stress_params[0];
  const Real q_ok = stress_params[1];
  setStressAfterReturn(stress_trial, p_ok, q_ok, gaE, intnl, smoothed_q, Eijkl, stress);
}

void
TwoParameterPlasticityStressUpdate::setInelasticStrainIncrementAfterReturn(
    const RankTwoTensor & /*stress_trial*/,
    Real gaE,
    const yieldAndFlow & smoothed_q,
    const RankFourTensor & /*elasticity_tensor*/,
    const RankTwoTensor & returned_stress,
    RankTwoTensor & inelastic_strain_increment) const
{
  inelastic_strain_increment = (gaE / _Epp) * (smoothed_q.dg[0] * dpdstress(returned_stress) +
                                               smoothed_q.dg[1] * dqdstress(returned_stress));
}

std::vector<RankTwoTensor>
TwoParameterPlasticityStressUpdate::dstress_param_dstress(const RankTwoTensor & stress) const
{
  std::vector<RankTwoTensor> dsp(_num_pq, RankTwoTensor());
  dsp[0] = dpdstress(stress);
  dsp[1] = dqdstress(stress);
  return dsp;
}

std::vector<RankFourTensor>
TwoParameterPlasticityStressUpdate::d2stress_param_dstress(const RankTwoTensor & stress) const
{
  std::vector<RankFourTensor> d2(_num_pq, RankFourTensor());
  d2[0] = d2pdstress2(stress);
  d2[1] = d2qdstress2(stress);
  return d2;
}
