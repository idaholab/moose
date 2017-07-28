/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "CappedMohrCoulombStressUpdate.h"
#include "libmesh/utility.h"

template <>
InputParameters
validParams<CappedMohrCoulombStressUpdate>()
{
  InputParameters params = validParams<MultiParameterPlasticityStressUpdate>();
  params.addRequiredParam<UserObjectName>(
      "tensile_strength",
      "A TensorMechanicsHardening UserObject that defines hardening of the "
      "tensile strength.  In physical situations this is positive (and always "
      "must be greater than negative compressive-strength.");
  params.addRequiredParam<UserObjectName>(
      "compressive_strength",
      "A TensorMechanicsHardening UserObject that defines hardening of the "
      "compressive strength.  In physical situations this is positive.");
  params.addRequiredParam<UserObjectName>(
      "cohesion", "A TensorMechanicsHardening UserObject that defines hardening of the cohesion");
  params.addRequiredParam<UserObjectName>("friction_angle",
                                          "A TensorMechanicsHardening UserObject "
                                          "that defines hardening of the "
                                          "friction angle (in radians)");
  params.addRequiredParam<UserObjectName>(
      "dilation_angle",
      "A TensorMechanicsHardening UserObject that defines hardening of the "
      "dilation angle (in radians).  Unless you are quite confident, this should "
      "be set positive and not greater than the friction angle.");
  params.addParam<bool>("perfect_guess",
                        true,
                        "Provide a guess to the Newton-Raphson proceedure "
                        "that is the result from perfect plasticity.  With "
                        "severe hardening/softening this may be "
                        "suboptimal.");
  params.addClassDescription("Nonassociative, smoothed, Mohr-Coulomb plasticity capped with "
                             "tensile (Rankine) and compressive caps, with hardening/softening");
  return params;
}

CappedMohrCoulombStressUpdate::CappedMohrCoulombStressUpdate(const InputParameters & parameters)
  : MultiParameterPlasticityStressUpdate(parameters, 3, 12, 2),
    _tensile_strength(getUserObject<TensorMechanicsHardeningModel>("tensile_strength")),
    _compressive_strength(getUserObject<TensorMechanicsHardeningModel>("compressive_strength")),
    _cohesion(getUserObject<TensorMechanicsHardeningModel>("cohesion")),
    _phi(getUserObject<TensorMechanicsHardeningModel>("friction_angle")),
    _psi(getUserObject<TensorMechanicsHardeningModel>("dilation_angle")),
    _perfect_guess(getParam<bool>("perfect_guess")),
    _poissons_ratio(0.0),
    _eigvecs(RankTwoTensor())
{
  if (_psi.value(0.0) <= 0.0 || _psi.value(0.0) > _phi.value(0.0))
    mooseWarning("Usually the Mohr-Coulomb dilation angle is positive and not greater than the "
                 "friction angle");
}

void
CappedMohrCoulombStressUpdate::computeStressParams(const RankTwoTensor & stress,
                                                   std::vector<Real> & stress_params) const
{
  // stress_params[0] = smallest eigenvalue, stress_params[2] = largest eigenvalue
  stress.symmetricEigenvalues(stress_params);
}

std::vector<RankTwoTensor>
CappedMohrCoulombStressUpdate::dstress_param_dstress(const RankTwoTensor & stress) const
{
  std::vector<Real> sp;
  std::vector<RankTwoTensor> dsp;
  stress.dsymmetricEigenvalues(sp, dsp);
  return dsp;
}

std::vector<RankFourTensor>
CappedMohrCoulombStressUpdate::d2stress_param_dstress(const RankTwoTensor & stress) const
{
  std::vector<RankFourTensor> d2;
  stress.d2symmetricEigenvalues(d2);
  return d2;
}

void
CappedMohrCoulombStressUpdate::preReturnMapV(const std::vector<Real> & /*trial_stress_params*/,
                                             const RankTwoTensor & stress_trial,
                                             const std::vector<Real> & /*intnl_old*/,
                                             const std::vector<Real> & /*yf*/,
                                             const RankFourTensor & Eijkl)
{
  std::vector<Real> eigvals;
  stress_trial.symmetricEigenvaluesEigenvectors(eigvals, _eigvecs);
  _poissons_ratio = Eijkl(2, 2, 0, 0) / (Eijkl(2, 2, 2, 2) + Eijkl(2, 2, 0, 0));
}

void
CappedMohrCoulombStressUpdate::setStressAfterReturnV(const RankTwoTensor & /*stress_trial*/,
                                                     const std::vector<Real> & stress_params,
                                                     Real /*gaE*/,
                                                     const std::vector<Real> & /*intnl*/,
                                                     const yieldAndFlow & /*smoothed_q*/,
                                                     const RankFourTensor & /*Eijkl*/,
                                                     RankTwoTensor & stress) const
{
  // form the diagonal stress
  stress = RankTwoTensor(stress_params[0], stress_params[1], stress_params[2], 0.0, 0.0, 0.0);
  // rotate to the original frame
  stress = _eigvecs * stress * (_eigvecs.transpose());
}

void
CappedMohrCoulombStressUpdate::yieldFunctionValuesV(const std::vector<Real> & stress_params,
                                                    const std::vector<Real> & intnl,
                                                    std::vector<Real> & yf) const
{
  // intnl[0] = shear, intnl[1] = tensile
  const Real ts = _tensile_strength.value(intnl[1]);
  const Real cs = _compressive_strength.value(intnl[1]);
  const Real sinphi = std::sin(_phi.value(intnl[0]));
  const Real cohcos = _cohesion.value(intnl[0]) * std::cos(_phi.value(intnl[0]));
  const Real smax = stress_params[2]; // largest eigenvalue
  const Real smid = stress_params[1];
  const Real smin = stress_params[0]; // smallest eigenvalue
  yf[0] = smax - ts;
  yf[1] = smid - ts;
  yf[2] = smin - ts;
  yf[3] = -smin - cs;
  yf[4] = -smid - cs;
  yf[5] = -smax - cs;
  yf[6] = 0.5 * (smax - smin) + 0.5 * (smax + smin) * sinphi - cohcos;
  yf[7] = 0.5 * (smid - smin) + 0.5 * (smid + smin) * sinphi - cohcos;
  yf[8] = 0.5 * (smax - smid) + 0.5 * (smax + smid) * sinphi - cohcos;
  yf[9] = 0.5 * (smid - smax) + 0.5 * (smid + smax) * sinphi - cohcos;
  yf[10] = 0.5 * (smin - smid) + 0.5 * (smin + smid) * sinphi - cohcos;
  yf[11] = 0.5 * (smin - smax) + 0.5 * (smin + smax) * sinphi - cohcos;
}

void
CappedMohrCoulombStressUpdate::computeAllQV(const std::vector<Real> & stress_params,
                                            const std::vector<Real> & intnl,
                                            std::vector<yieldAndFlow> & all_q) const
{
  const Real ts = _tensile_strength.value(intnl[1]);
  const Real dts = _tensile_strength.derivative(intnl[1]);
  const Real cs = _compressive_strength.value(intnl[1]);
  const Real dcs = _compressive_strength.derivative(intnl[1]);
  const Real sinphi = std::sin(_phi.value(intnl[0]));
  const Real dsinphi = std::cos(_phi.value(intnl[0])) * _phi.derivative(intnl[0]);
  const Real sinpsi = std::sin(_psi.value(intnl[0]));
  const Real dsinpsi = std::cos(_psi.value(intnl[0])) * _psi.derivative(intnl[0]);
  const Real cohcos = _cohesion.value(intnl[0]) * std::cos(_phi.value(intnl[0]));
  const Real dcohcos =
      _cohesion.derivative(intnl[0]) * std::cos(_phi.value(intnl[0])) -
      _cohesion.value(intnl[0]) * std::sin(_phi.value(intnl[0])) * _phi.derivative(intnl[0]);
  const Real smax = stress_params[2]; // largest eigenvalue
  const Real smid = stress_params[1];
  const Real smin = stress_params[0]; // smallest eigenvalue

  // yield functions.  See comment in yieldFunctionValuesV
  all_q[0].f = smax - ts;
  all_q[1].f = smid - ts;
  all_q[2].f = smin - ts;
  all_q[3].f = -smin - cs;
  all_q[4].f = -smid - cs;
  all_q[5].f = -smax - cs;
  all_q[6].f = 0.5 * (smax - smin) + 0.5 * (smax + smin) * sinphi - cohcos;
  all_q[7].f = 0.5 * (smid - smin) + 0.5 * (smid + smin) * sinphi - cohcos;
  all_q[8].f = 0.5 * (smax - smid) + 0.5 * (smax + smid) * sinphi - cohcos;
  all_q[9].f = 0.5 * (smid - smax) + 0.5 * (smid + smax) * sinphi - cohcos;
  all_q[10].f = 0.5 * (smin - smid) + 0.5 * (smin + smid) * sinphi - cohcos;
  all_q[11].f = 0.5 * (smin - smax) + 0.5 * (smin + smax) * sinphi - cohcos;

  // d(yield function)/d(stress_params)
  for (unsigned yf = 0; yf < _num_yf; ++yf)
    for (unsigned a = 0; a < _num_sp; ++a)
      all_q[yf].df[a] = 0.0;
  all_q[0].df[2] = 1.0;
  all_q[1].df[1] = 1.0;
  all_q[2].df[0] = 1.0;
  all_q[3].df[0] = -1.0;
  all_q[4].df[1] = -1.0;
  all_q[5].df[2] = -1.0;
  all_q[6].df[2] = 0.5 * (1.0 + sinphi);
  all_q[6].df[0] = 0.5 * (-1.0 + sinphi);
  all_q[7].df[1] = 0.5 * (1.0 + sinphi);
  all_q[7].df[0] = 0.5 * (-1.0 + sinphi);
  all_q[8].df[2] = 0.5 * (1.0 + sinphi);
  all_q[8].df[1] = 0.5 * (-1.0 + sinphi);
  all_q[9].df[1] = 0.5 * (1.0 + sinphi);
  all_q[9].df[2] = 0.5 * (-1.0 + sinphi);
  all_q[10].df[0] = 0.5 * (1.0 + sinphi);
  all_q[10].df[1] = 0.5 * (-1.0 + sinphi);
  all_q[11].df[0] = 0.5 * (1.0 + sinphi);
  all_q[11].df[2] = 0.5 * (-1.0 + sinphi);

  // d(yield function)/d(intnl)
  for (unsigned i = 0; i < 6; ++i)
    all_q[i].df_di[0] = 0.0;
  all_q[0].df_di[1] = all_q[1].df_di[1] = all_q[2].df_di[1] = -dts;
  all_q[3].df_di[1] = all_q[4].df_di[1] = all_q[5].df_di[1] = -dcs;
  for (unsigned i = 6; i < 12; ++i)
    all_q[i].df_di[1] = 0.0;
  all_q[6].df_di[0] = 0.5 * (smax + smin) * dsinphi - dcohcos;
  all_q[7].df_di[0] = 0.5 * (smid + smin) * dsinphi - dcohcos;
  all_q[8].df_di[0] = 0.5 * (smax + smid) * dsinphi - dcohcos;
  all_q[9].df_di[0] = 0.5 * (smid + smax) * dsinphi - dcohcos;
  all_q[10].df_di[0] = 0.5 * (smin + smid) * dsinphi - dcohcos;
  all_q[11].df_di[0] = 0.5 * (smin + smax) * dsinphi - dcohcos;

  // the flow potential is just the yield function with phi->psi
  // d(flow potential)/d(stress_params)
  for (unsigned yf = 0; yf < 6; ++yf)
    for (unsigned a = 0; a < _num_sp; ++a)
      all_q[yf].dg[a] = all_q[yf].df[a];
  all_q[6].dg[2] = all_q[7].dg[1] = all_q[8].dg[2] = all_q[9].dg[1] = all_q[10].dg[0] =
      all_q[11].dg[0] = 0.5 * (1.0 + sinpsi);
  all_q[6].dg[0] = all_q[7].dg[0] = all_q[8].dg[1] = all_q[9].dg[2] = all_q[10].dg[1] =
      all_q[11].dg[2] = 0.5 * (-1.0 + sinpsi);

  // d(flow potential)/d(stress_params)/d(intnl)
  for (unsigned yf = 0; yf < _num_yf; ++yf)
    for (unsigned a = 0; a < _num_sp; ++a)
      for (unsigned i = 0; i < _num_intnl; ++i)
        all_q[yf].d2g_di[a][i] = 0.0;
  all_q[6].d2g_di[2][0] = all_q[7].d2g_di[1][0] = all_q[8].d2g_di[2][0] = all_q[9].d2g_di[1][0] =
      all_q[10].d2g_di[0][0] = all_q[11].d2g_di[0][0] = 0.5 * dsinpsi;
  all_q[6].d2g_di[0][0] = all_q[7].d2g_di[0][0] = all_q[8].d2g_di[1][0] = all_q[9].d2g_di[2][0] =
      all_q[10].d2g_di[1][0] = all_q[11].d2g_di[2][0] = 0.5 * dsinpsi;

  // d(flow potential)/d(stress_params)/d(stress_params)
  for (unsigned yf = 0; yf < _num_yf; ++yf)
    for (unsigned a = 0; a < _num_sp; ++a)
      for (unsigned b = 0; b < _num_sp; ++b)
        all_q[yf].d2g[a][b] = 0.0;
}

void
CappedMohrCoulombStressUpdate::setEffectiveElasticity(const RankFourTensor & Eijkl)
{
  // Eijkl is required to be isotropic, so we can use the
  // frame where stress is diagonal
  for (unsigned a = 0; a < _num_sp; ++a)
    for (unsigned b = 0; b < _num_sp; ++b)
      _Eij[a][b] = Eijkl(a, a, b, b);
  _En = _Eij[2][2];
  const Real denom = _Eij[0][0] * (_Eij[0][0] + _Eij[0][1]) - 2 * Utility::pow<2>(_Eij[0][1]);
  for (unsigned a = 0; a < _num_sp; ++a)
  {
    _Cij[a][a] = (_Eij[0][0] + _Eij[0][1]) / denom;
    for (unsigned b = 0; b < a; ++b)
      _Cij[a][b] = _Cij[b][a] = -_Eij[0][1] / denom;
  }
}

void
CappedMohrCoulombStressUpdate::initialiseVarsV(const std::vector<Real> & trial_stress_params,
                                               const std::vector<Real> & intnl_old,
                                               std::vector<Real> & stress_params,
                                               Real & gaE,
                                               std::vector<Real> & intnl) const
{
  if (!_perfect_guess)
  {
    for (unsigned i = 0; i < _num_sp; ++i)
      stress_params[i] = trial_stress_params[i];
    gaE = 0.0;
  }
  else
  {
    const Real ts = _tensile_strength.value(intnl_old[1]);
    const Real cs = _compressive_strength.value(intnl_old[1]);
    const Real sinphi = std::sin(_phi.value(intnl_old[0]));
    const Real sinpsi = std::sin(_psi.value(intnl_old[0]));
    const Real cohcos = _cohesion.value(intnl_old[0]) * std::cos(_phi.value(intnl_old[0]));
    bool found_solution = false;

    if (stress_params[2] - ts > 0.0)
    {
      // try pure tensile failure
      stress_params[2] = ts; // largest eigenvalue
      stress_params[1] = std::min(stress_params[1], ts);
      stress_params[0] = std::min(stress_params[0], ts);
      gaE = trial_stress_params[2] - stress_params[2];
      const Real mc_yf = 0.5 * (stress_params[2] - stress_params[0]) +
                         0.5 * (stress_params[2] + stress_params[0]) * sinphi - cohcos;
      if (mc_yf <= 0.0 && -stress_params[0] - cs <= 0.0)
        found_solution = true;
    }
    if (!found_solution && -stress_params[0] - cs > 0.0)
    {
      // try pure compressive failure
      stress_params[0] = -cs; // largest eigenvalue
      stress_params[1] = std::max(stress_params[1], -cs);
      stress_params[2] = std::max(stress_params[2], -cs);
      gaE = -trial_stress_params[0] + stress_params[0];
      const Real mc_yf = 0.5 * (stress_params[2] - stress_params[0]) +
                         0.5 * (stress_params[2] + stress_params[0]) * sinphi - cohcos;
      if (mc_yf <= 0.0 && stress_params[2] - ts <= 0.0)
        found_solution = true;
    }
    if (!found_solution)
    {
      // try pure shear failure
      const Real ga = ((trial_stress_params[2] - trial_stress_params[0]) +
                       (trial_stress_params[2] + trial_stress_params[0]) * sinphi - 2.0 * cohcos) /
                      (_Eij[2][2] - _Eij[0][2] + (_Eij[2][2] + _Eij[0][2]) * sinpsi * sinphi);
      stress_params[2] = trial_stress_params[2] -
                         0.5 * ga * (_Eij[2][2] * (1.0 + sinpsi) + _Eij[2][0] * (-1.0 + sinpsi));
      stress_params[1] = trial_stress_params[1] - ga * _Eij[1][0] * sinpsi;
      stress_params[0] = trial_stress_params[0] -
                         0.5 * ga * (_Eij[0][0] * (-1.0 + sinpsi) + _Eij[0][2] * (1.0 + sinpsi));
      if (stress_params[0] >= stress_params[1] && stress_params[1] >= stress_params[2])
        // tip
        stress_params[0] = stress_params[1] = stress_params[2] = cohcos / sinphi;
      stress_params[1] = std::max(std::min(stress_params[1], stress_params[2]), stress_params[0]);
      gaE = ((trial_stress_params[2] - trial_stress_params[0]) -
             (stress_params[2] - stress_params[0])) /
            (_Eij[2][2] - _Eij[0][2]) * _Eij[2][2];
      if (stress_params[2] - ts <= 0.0 && -stress_params[0] - cs <= 0.0)
        found_solution = true;
    }
    if (!found_solution)
    {
      // must be mixed return.  Just use
      for (unsigned i = 0; i < _num_sp; ++i)
        stress_params[i] = trial_stress_params[i];
      gaE = 0.0;
    }
  }
  setIntnlValuesV(trial_stress_params, stress_params, intnl_old, intnl);
}

void
CappedMohrCoulombStressUpdate::setIntnlValuesV(const std::vector<Real> & trial_stress_params,
                                               const std::vector<Real> & current_stress_params,
                                               const std::vector<Real> & intnl_old,
                                               std::vector<Real> & intnl) const
{
  // intnl[0] = shear, intnl[1] = tensile
  const Real smax = current_stress_params[2];     // largest eigenvalue
  const Real smin = current_stress_params[0];     // smallest eigenvalue
  const Real trial_smax = trial_stress_params[2]; // largest eigenvalue
  const Real trial_smin = trial_stress_params[0]; // smallest eigenvalue
  const Real ga_shear = ((trial_smax - trial_smin) - (smax - smin)) / (_Eij[2][2] - _Eij[0][2]);
  intnl[0] = intnl_old[0] + ga_shear;
  const Real sinpsi = std::sin(_psi.value(intnl[0]));
  const Real prefactor = (_Eij[2][2] + _Eij[0][2]) * sinpsi;
  const Real shear_correction = prefactor * ga_shear;
  const Real ga_tensile = (1.0 - _poissons_ratio) *
                          ((trial_smax + trial_smin) - (smax + smin) - shear_correction) /
                          _Eij[2][2];
  intnl[1] = intnl_old[1] + ga_tensile;
}

void
CappedMohrCoulombStressUpdate::setIntnlDerivativesV(const std::vector<Real> & trial_stress_params,
                                                    const std::vector<Real> & current_stress_params,
                                                    const std::vector<Real> & intnl,
                                                    std::vector<std::vector<Real>> & dintnl) const
{
  // intnl[0] = shear, intnl[1] = tensile
  const Real smax = current_stress_params[2];     // largest eigenvalue
  const Real smin = current_stress_params[0];     // smallest eigenvalue
  const Real trial_smax = trial_stress_params[2]; // largest eigenvalue
  const Real trial_smin = trial_stress_params[0]; // smallest eigenvalue
  const Real ga_shear = ((trial_smax - trial_smin) - (smax - smin)) / (_Eij[2][2] - _Eij[0][2]);
  const std::vector<Real> dga_shear = {
      1.0 / (_Eij[2][2] - _Eij[0][2]), 0.0, -1.0 / (_Eij[2][2] - _Eij[0][2])};
  // intnl[0] = intnl_old[0] + ga_shear;
  for (std::size_t i = 0; i < _num_sp; ++i)
    dintnl[0][i] = dga_shear[i];

  const Real sinpsi = std::sin(_psi.value(intnl[0]));
  const Real dsinpsi_di0 = _psi.derivative(intnl[0]) * std::cos(_psi.value(intnl[0]));

  const Real prefactor = (_Eij[2][2] + _Eij[0][2]) * sinpsi;
  const Real dprefactor_di0 = (_Eij[2][2] + _Eij[0][2]) * dsinpsi_di0;
  // const Real shear_correction = prefactor * ga_shear;
  std::vector<Real> dshear_correction(_num_sp);
  for (std::size_t i = 0; i < _num_sp; ++i)
    dshear_correction[i] = prefactor * dga_shear[i] + dprefactor_di0 * dintnl[0][i] * ga_shear;
  // const Real ga_tensile = (1 - _poissons_ratio) * ((trial_smax + trial_smin) - (smax + smin) -
  // shear_correction) /
  // _Eij[2][2];
  // intnl[1] = intnl_old[1] + ga_tensile;
  for (std::size_t i = 0; i < _num_sp; ++i)
    dintnl[1][i] = -(1.0 - _poissons_ratio) * dshear_correction[i] / _Eij[2][2];
  dintnl[1][2] += -(1.0 - _poissons_ratio) / _Eij[2][2];
  dintnl[1][0] += -(1.0 - _poissons_ratio) / _Eij[2][2];
}

void
CappedMohrCoulombStressUpdate::consistentTangentOperatorV(
    const RankTwoTensor & stress_trial,
    const std::vector<Real> & trial_stress_params,
    const RankTwoTensor & /*stress*/,
    const std::vector<Real> & stress_params,
    Real /*gaE*/,
    const yieldAndFlow & /*smoothed_q*/,
    const RankFourTensor & elasticity_tensor,
    bool compute_full_tangent_operator,
    const std::vector<std::vector<Real>> & dvar_dtrial,
    RankFourTensor & cto)
{
  if (!_fe_problem.currentlyComputingJacobian())
    return;

  cto = elasticity_tensor;
  if (!compute_full_tangent_operator)
    return;

  // dvar_dtrial has been computed already, so
  // d(stress)/d(trial_stress) = d(eigvecs * stress_params * eigvecs.transpose())/d(trial_stress)
  // eigvecs is a rotation matrix, rot(i, j) = e_j(i) = i^th component of j^th eigenvector
  // d(rot_ij)/d(stress_kl) = d(e_j(i))/d(stress_kl)
  // = sum_a 0.5 * e_a(i) * (e_a(k)e_j(l) + e_a(l)e_j(k)) / (la_j - la_a)
  // = sum_a 0.5 * rot(i,a) * (rot(k,a)rot(l,j) + rot(l,a)*rot(k,j)) / (la_j - la_a)
  RankFourTensor drot_dstress;
  for (unsigned i = 0; i < _tensor_dimensionality; ++i)
    for (unsigned j = 0; j < _tensor_dimensionality; ++j)
      for (unsigned k = 0; k < _tensor_dimensionality; ++k)
        for (unsigned l = 0; l < _tensor_dimensionality; ++l)
          for (unsigned a = 0; a < _num_sp; ++a)
          {
            if (trial_stress_params[a] == trial_stress_params[j])
              continue;
            drot_dstress(i, j, k, l) += 0.5 * _eigvecs(i, a) * (_eigvecs(k, a) * _eigvecs(l, j) +
                                                                _eigvecs(l, a) * _eigvecs(k, j)) /
                                        (trial_stress_params[j] - trial_stress_params[a]);
          }

  const RankTwoTensor eT = _eigvecs.transpose();

  RankFourTensor dstress_dtrial;
  for (unsigned i = 0; i < _tensor_dimensionality; ++i)
    for (unsigned j = 0; j < _tensor_dimensionality; ++j)
      for (unsigned k = 0; k < _tensor_dimensionality; ++k)
        for (unsigned l = 0; l < _tensor_dimensionality; ++l)
          for (unsigned a = 0; a < _num_sp; ++a)
            dstress_dtrial(i, j, k, l) +=
                drot_dstress(i, a, k, l) * stress_params[a] * eT(a, j) +
                _eigvecs(i, a) * stress_params[a] * drot_dstress(j, a, k, l);

  const std::vector<RankTwoTensor> dsp_trial = dstress_param_dstress(stress_trial);
  for (unsigned i = 0; i < _tensor_dimensionality; ++i)
    for (unsigned j = 0; j < _tensor_dimensionality; ++j)
      for (unsigned k = 0; k < _tensor_dimensionality; ++k)
        for (unsigned l = 0; l < _tensor_dimensionality; ++l)
          for (unsigned a = 0; a < _num_sp; ++a)
            for (unsigned b = 0; b < _num_sp; ++b)
              dstress_dtrial(i, j, k, l) +=
                  _eigvecs(i, a) * dvar_dtrial[a][b] * dsp_trial[b](k, l) * eT(a, j);

  cto = dstress_dtrial * elasticity_tensor;
}
