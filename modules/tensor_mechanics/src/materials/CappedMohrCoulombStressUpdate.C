//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CappedMohrCoulombStressUpdate.h"
#include "libmesh/utility.h"
#include "ElasticityTensorTools.h"

registerMooseObject("TensorMechanicsApp", CappedMohrCoulombStressUpdate);

InputParameters
CappedMohrCoulombStressUpdate::validParams()
{
  InputParameters params = MultiParameterPlasticityStressUpdate::validParams();
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
                        "Provide a guess to the Newton-Raphson procedure "
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
    _shifter(_f_tol),
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
  _poissons_ratio = ElasticityTensorTools::getIsotropicPoissonsRatio(Eijkl);
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
CappedMohrCoulombStressUpdate::initializeVarsV(const std::vector<Real> & trial_stress_params,
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
    const Real cohcos = _cohesion.value(intnl_old[0]) * std::cos(_phi.value(intnl_old[0]));

    const Real trial_tensile_yf = trial_stress_params[2] - ts;
    const Real trial_compressive_yf = -trial_stress_params[0] - cs;
    const Real trial_mc_yf = 0.5 * (trial_stress_params[2] - trial_stress_params[0]) +
                             0.5 * (trial_stress_params[2] + trial_stress_params[0]) * sinphi -
                             cohcos;

    /**
     * For CappedMohrCoulomb there are a number of possibilities for the
     * returned stress configuration:
     * - return to the Tensile yield surface to its plane
     * - return to the Tensile yield surface to its max=mid edge
     * - return to the Tensile yield surface to its tip
     * - return to the Compressive yield surface to its plane
     * - return to the Compressive yield surface to its max=mid edge
     * - return to the Compressive  yield surface to its tip
     * - return to the Mohr-Coulomb yield surface to its plane
     * - return to the Mohr-Coulomb yield surface to its max=mid edge
     * - return to the Mohr-Coulomb yield surface to its mid=min edge
     * - return to the Mohr-Coulomb yield surface to its tip
     * - return to the edge between Tensile and Mohr-Coulomb
     * - return to the edge between Tensile and Mohr-Coulomb, at max=mid point
     * - return to the edge between Tensile and Mohr-Coulomb, at mid=min point
     * - return to the edge between Compressive and Mohr-Coulomb
     * - return to the edge between Compressive and Mohr-Coulomb, at max=mid point
     * - return to the edge between Compressive and Mohr-Coulomb, at mid=min point
     * Which of these is more prelevant depends on the parameters
     * tensile strength, compressive strength, cohesion, etc.
     * I simply check each possibility until i find one that works.
     * _shifter is used to avoid equal eigenvalues
     */

    bool found_solution = false;

    if (trial_tensile_yf <= _f_tol && trial_compressive_yf <= _f_tol && trial_mc_yf <= _f_tol)
    {
      // this is needed because although we know the smoothed yield function is
      // positive, the individual yield functions may not be
      for (unsigned i = 0; i < _num_sp; ++i)
        stress_params[i] = trial_stress_params[i];
      gaE = 0.0;
      found_solution = true;
    }

    const bool tensile_possible = (ts < cohcos / sinphi); // tensile chops MC tip
    const bool mc_tip_possible = (cohcos / sinphi < ts);  // MC tip pokes through tensile
    const bool mc_impossible = (0.5 * (ts + cs) + 0.5 * (ts - cs) * sinphi - cohcos <
                                _f_tol); // MC outside tensile and compressive

    const Real sinpsi = std::sin(_psi.value(intnl_old[0]));
    const Real halfplus = 0.5 + 0.5 * sinpsi;
    const Real neghalfplus = -0.5 + 0.5 * sinpsi;

    if (!found_solution && tensile_possible && trial_tensile_yf > _f_tol &&
        (trial_compressive_yf <= _f_tol || (trial_compressive_yf > _f_tol && mc_impossible)))
    {
      // try pure tensile failure, return to the plane
      // This involves solving yf[0] = 0 and the three flow-direction equations
      // Don't try this if there is compressive failure, since returning to
      // the tensile yield surface will only make compressive failure worse
      const Real ga = (trial_stress_params[2] - ts) / _Eij[2][2];
      stress_params[2] = ts; // largest eigenvalue
      stress_params[1] = trial_stress_params[1] - ga * _Eij[1][2];
      stress_params[0] = trial_stress_params[0] - ga * _Eij[0][2];

      // if we have to return to the edge, or tip, do that
      Real dist_mod = 1.0;
      const Real to_subtract1 = stress_params[1] - (ts - 0.5 * _shifter);
      if (to_subtract1 > 0.0)
      {
        dist_mod += Utility::pow<2>(to_subtract1 / (trial_stress_params[2] - ts));
        stress_params[1] -= to_subtract1;
      }
      const Real to_subtract0 = stress_params[0] - (ts - _shifter);
      if (to_subtract0 > 0.0)
      {
        dist_mod += Utility::pow<2>(to_subtract0 / (trial_stress_params[2] - ts));
        stress_params[0] -= to_subtract0;
      }
      if (mc_impossible) // might have to shift up to the compressive yield surface
      {
        const Real to_add0 = -stress_params[0] - cs;
        if (to_add0 > 0.0)
        {
          dist_mod += Utility::pow<2>(to_add0 / (trial_stress_params[2] - ts));
          stress_params[0] += to_add0;
        }
        const Real to_add1 = -cs + 0.5 * _shifter - stress_params[1];
        if (to_add1 > 0.0)
        {
          dist_mod += Utility::pow<2>(to_add1 / (trial_stress_params[2] - ts));
          stress_params[1] += to_add1;
        }
      }

      const Real new_compressive_yf = -stress_params[0] - cs;
      const Real new_mc_yf = 0.5 * (stress_params[2] - stress_params[0]) +
                             0.5 * (stress_params[2] + stress_params[0]) * sinphi - cohcos;
      if (new_mc_yf <= _f_tol && new_compressive_yf <= _f_tol)
      {
        gaE = std::sqrt(dist_mod) * (trial_stress_params[2] - stress_params[2]);
        found_solution = true;
      }
    }
    if (!found_solution && trial_compressive_yf > _f_tol &&
        (trial_tensile_yf <= _f_tol || (trial_tensile_yf > _f_tol && mc_impossible)))
    {
      // try pure compressive failure
      // Don't try this if there is tensile failure, since returning to
      // the compressive yield surface will only make tensile failure worse
      const Real ga = (trial_stress_params[0] + cs) / _Eij[0][0]; // this is negative
      stress_params[0] = -cs;
      stress_params[1] = trial_stress_params[1] - ga * _Eij[1][0];
      stress_params[2] = trial_stress_params[2] - ga * _Eij[2][0];

      // if we have to return to the edge, or tip, do that
      Real dist_mod = 1.0;
      const Real to_add1 = -cs + 0.5 * _shifter - stress_params[1];
      if (to_add1 > 0.0)
      {
        dist_mod += Utility::pow<2>(to_add1 / (trial_stress_params[0] + cs));
        stress_params[1] += to_add1;
      }
      const Real to_add2 = -cs + _shifter - stress_params[2];
      if (to_add2 > 0.0)
      {
        dist_mod += Utility::pow<2>(to_add2 / (trial_stress_params[0] + cs));
        stress_params[2] += to_add2;
      }
      if (mc_impossible) // might have to shift down to the tensile yield surface
      {
        const Real to_subtract2 = stress_params[2] - ts;
        if (to_subtract2 > 0.0)
        {
          dist_mod += Utility::pow<2>(to_subtract2 / (trial_stress_params[0] + cs));
          stress_params[2] -= to_subtract2;
        }
        const Real to_subtract1 = stress_params[1] - (ts - 0.5 * _shifter);
        if (to_subtract1 > 0.0)
        {
          dist_mod += Utility::pow<2>(to_subtract1 / (trial_stress_params[0] + cs));
          stress_params[1] -= to_subtract1;
        }
      }

      const Real new_tensile_yf = stress_params[2] - ts;
      const Real new_mc_yf = 0.5 * (stress_params[2] - stress_params[0]) +
                             0.5 * (stress_params[2] + stress_params[0]) * sinphi - cohcos;
      if (new_mc_yf <= _f_tol && new_tensile_yf <= _f_tol)
      {
        gaE = std::sqrt(dist_mod) * (-trial_stress_params[0] + stress_params[0]);
        found_solution = true;
      }
    }
    if (!found_solution && !mc_impossible && trial_mc_yf > _f_tol)
    {
      // try pure shear failure, return to the plane
      // This involves solving yf[6]=0 and the three flow-direction equations
      const Real ga = ((trial_stress_params[2] - trial_stress_params[0]) +
                       (trial_stress_params[2] + trial_stress_params[0]) * sinphi - 2.0 * cohcos) /
                      (_Eij[2][2] - _Eij[0][2] + (_Eij[2][2] + _Eij[0][2]) * sinpsi * sinphi);
      stress_params[2] =
          trial_stress_params[2] - ga * (_Eij[2][2] * halfplus + _Eij[2][0] * neghalfplus);
      stress_params[1] = trial_stress_params[1] - ga * _Eij[1][0] * sinpsi;
      stress_params[0] =
          trial_stress_params[0] - ga * (_Eij[0][0] * neghalfplus + _Eij[0][2] * halfplus);
      const Real f7 = 0.5 * (stress_params[1] - stress_params[0]) +
                      0.5 * (stress_params[1] + stress_params[0]) * sinphi - cohcos;
      const Real f8 = 0.5 * (stress_params[2] - stress_params[1]) +
                      0.5 * (stress_params[2] + stress_params[1]) * sinphi - cohcos;
      const Real new_tensile_yf = stress_params[2] - ts;
      const Real new_compressive_yf = -stress_params[0] - cs;

      if (f7 <= _f_tol && f8 <= _f_tol && new_tensile_yf <= _f_tol && new_compressive_yf <= _f_tol)
      {
        gaE = ((trial_stress_params[2] - trial_stress_params[0]) -
               (stress_params[2] - stress_params[0])) /
              (_Eij[2][2] - _Eij[0][2]) * _Eij[2][2];
        found_solution = true;
      }
    }
    if (!found_solution && !mc_impossible && trial_mc_yf > _f_tol)
    {
      // Try return to the max=mid MC line.
      // To return to the max=mid line, we need to solve f6 = 0 = f7 and
      // the three flow-direction equations.  In the flow-direction equations
      // there are two plasticity multipliers, which i call ga6 and ga7,
      // corresponding to the amounts of strain normal to the f6 and f7
      // directions, respectively.
      // So:
      // Smax = Smax^trial - ga6 Emax,a dg6/dSa - ga7 Emax,a dg7/dSa
      //      = Smax^trial - ga6 cmax6 - ga7 cmax7  (with cmax6 and cmax7 evaluated below)
      // Smid = Smid^trial - ga6 Emid,a dg6/dSa - ga7 Emid,a dg7/dSa
      //      = Smid^trial - ga6 cmid6 - ga7 cmid7
      // Smin = Smin^trial - ga6 Emin,a dg6/dSa - ga7 Emin,a dg7/dSa
      //      = Smin^trial - ga6 cmin6 - ga7 cmin7
      const Real cmax6 = _Eij[2][2] * halfplus + _Eij[2][0] * neghalfplus;
      const Real cmax7 = _Eij[2][1] * halfplus + _Eij[2][0] * neghalfplus;
      // const Real cmid6 = _Eij[1][2] * halfplus + _Eij[1][0] * neghalfplus;
      // const Real cmid7 = _Eij[1][1] * halfplus + _Eij[1][0] * neghalfplus;
      const Real cmin6 = _Eij[0][2] * halfplus + _Eij[0][0] * neghalfplus;
      const Real cmin7 = _Eij[0][1] * halfplus + _Eij[0][0] * neghalfplus;
      // Substituting these into f6 = 0 yields
      // 0 = f6_trial - ga6 (0.5(cmax6 - cmin6) + 0.5(cmax6 + cmin6)sinphi) - ga7 (0.5(cmax6 -
      // cmin6) + 0.5(cmax6 + cmin6)sinphi) = f6_trial - ga6 c6 - ga7 c7, where
      const Real c6 = 0.5 * (cmax6 - cmin6) + 0.5 * (cmax6 + cmin6) * sinphi;
      const Real c7 = 0.5 * (cmax7 - cmin7) + 0.5 * (cmax7 + cmin7) * sinphi;
      // It isn't too hard to check that the other equation is
      // 0 = f7_trial - ga6 c7 - ga7 c6
      // These equations may be inverted to yield
      if (c6 != c7)
      {
        const Real f6_trial = trial_mc_yf;
        const Real f7_trial = 0.5 * (trial_stress_params[1] - trial_stress_params[0]) +
                              0.5 * (trial_stress_params[1] + trial_stress_params[0]) * sinphi -
                              cohcos;
        const Real descr = Utility::pow<2>(c6) - Utility::pow<2>(c7);
        Real ga6 = (c6 * f6_trial - c7 * f7_trial) / descr;
        Real ga7 = (-c7 * f6_trial + c6 * f7_trial) / descr;
        // and finally
        stress_params[2] = trial_stress_params[2] - ga6 * cmax6 - ga7 * cmax7;
        stress_params[0] = trial_stress_params[0] - ga6 * cmin6 - ga7 * cmin7;
        stress_params[1] = stress_params[2] - 0.5 * _shifter;

        Real f8 = 0.5 * (stress_params[2] - stress_params[1]) +
                  0.5 * (stress_params[2] + stress_params[1]) * sinphi - cohcos;

        if (mc_tip_possible && f8 > _f_tol)
        {
          stress_params[2] = cohcos / sinphi;
          stress_params[1] = stress_params[2] - 0.5 * _shifter;
          stress_params[0] = stress_params[2] - _shifter;
          f8 = 0.0;
          ga6 = 1.0;
          ga7 = 1.0;
        }

        const Real new_tensile_yf = stress_params[2] - ts;
        const Real new_compressive_yf = -stress_params[0] - cs;

        if (f8 <= _f_tol && new_tensile_yf <= _f_tol && new_compressive_yf <= _f_tol &&
            ga6 >= 0.0 && ga7 >= 0.0)
        {
          gaE = ((trial_stress_params[2] - trial_stress_params[0]) -
                 (stress_params[2] - stress_params[0])) /
                (_Eij[2][2] - _Eij[0][2]) * _Eij[2][2];
          found_solution = true;
        }
      }
    }
    if (!found_solution && !mc_impossible && trial_mc_yf > _f_tol)
    {
      // Try return to the mid=min line.
      // To return to the mid=min line, we need to solve f6 = 0 = f8 and
      // the three flow-direction equations.  In the flow-direction equations
      // there are two plasticity multipliers, which i call ga6 and ga8,
      // corresponding to the amounts of strain normal to the f6 and f8
      // directions, respectively.
      // So:
      // Smax = Smax^trial - ga6 Emax,a dg6/dSa - ga8 Emax,a dg8/dSa
      //      = Smax^trial - ga6 cmax6 - ga8 cmax8  (with cmax6 and cmax8 evaluated below)
      // Smid = Smid^trial - ga6 Emid,a dg6/dSa - ga8 Emid,a dg8/dSa
      //      = Smid^trial - ga6 cmid6 - ga8 cmid8
      // Smin = Smin^trial - ga6 Emin,a dg6/dSa - ga8 Emin,a dg8/dSa
      //      = Smin^trial - ga6 cmin6 - ga8 cmin8
      const Real cmax6 = _Eij[2][2] * halfplus + _Eij[2][0] * neghalfplus;
      const Real cmax8 = _Eij[2][2] * halfplus + _Eij[2][1] * neghalfplus;
      // const Real cmid6 = _Eij[1][2] * halfplus + _Eij[1][0] * neghalfplus;
      // const Real cmid8 = _Eij[1][2] * halfplus + _Eij[1][1] * neghalfplus;
      const Real cmin6 = _Eij[0][2] * halfplus + _Eij[0][0] * neghalfplus;
      const Real cmin8 = _Eij[0][2] * halfplus + _Eij[0][1] * neghalfplus;
      // Substituting these into f6 = 0 yields
      // 0 = f6_trial - ga6 (0.5(cmax6 - cmin6) + 0.5(cmax6 + cmin6)sinphi) - ga8 (0.5(cmax6 -
      // cmin6) + 0.5(cmax6 + cmin6)sinphi) = f6_trial - ga6 c6 - ga8 c8, where
      const Real c6 = 0.5 * (cmax6 - cmin6) + 0.5 * (cmax6 + cmin6) * sinphi;
      const Real c8 = 0.5 * (cmax8 - cmin8) + 0.5 * (cmax8 + cmin8) * sinphi;
      // It isn't too hard to check that the other equation is
      // 0 = f8_trial - ga6 c8 - ga8 c6
      // These equations may be inverted to yield
      if (c6 != c8)
      {
        const Real f6_trial = trial_mc_yf;
        const Real f8_trial = 0.5 * (trial_stress_params[2] - trial_stress_params[1]) +
                              0.5 * (trial_stress_params[2] + trial_stress_params[1]) * sinphi -
                              cohcos;
        const Real descr = Utility::pow<2>(c6) - Utility::pow<2>(c8);
        Real ga6 = (c6 * f6_trial - c8 * f8_trial) / descr;
        Real ga8 = (-c8 * f6_trial + c6 * f8_trial) / descr;
        // and finally
        stress_params[2] = trial_stress_params[2] - ga6 * cmax6 - ga8 * cmax8;
        stress_params[0] = trial_stress_params[0] - ga6 * cmin6 - ga8 * cmin8;
        stress_params[1] = stress_params[0] + 0.5 * _shifter;

        Real f7 = 0.5 * (stress_params[1] - stress_params[0]) +
                  0.5 * (stress_params[1] + stress_params[0]) * sinphi - cohcos;

        if (mc_tip_possible && f7 > _f_tol)
        {
          stress_params[2] = cohcos / sinphi;
          stress_params[1] = stress_params[2] - 0.5 * _shifter;
          stress_params[0] = stress_params[2] - _shifter;
          f7 = 0.0;
          ga6 = 1.0;
          ga8 = 1.0;
        }

        const Real new_tensile_yf = stress_params[2] - ts;
        const Real new_compressive_yf = -stress_params[0] - cs;

        if (f7 <= _f_tol && new_tensile_yf <= _f_tol && new_compressive_yf <= _f_tol &&
            ga6 >= 0.0 && ga8 >= 0.0)
        {
          gaE = ((trial_stress_params[2] - trial_stress_params[0]) -
                 (stress_params[2] - stress_params[0])) /
                (_Eij[2][2] - _Eij[0][2]) * _Eij[2][2];
          found_solution = true;
        }
      }
    }
    if (!found_solution && !mc_impossible && tensile_possible && trial_tensile_yf > _f_tol)
    {
      // Return to the line where yf[0] = 0 = yf[6].
      // To return to this line, we need to solve f0 = 0 = f6 and
      // the three flow-direction equations.  In the flow-direction equations
      // there are two plasticity multipliers, which i call ga0 and ga6
      // corresponding to the amounts of strain normal to the f0 and f6
      // directions, respectively.
      // So:
      // Smax = Smax^trial - ga6 Emax,a dg6/dSa - ga0 Emax,a dg0/dSa
      //      = Smax^trial - ga6 cmax6 - ga0 cmax0  (with cmax6 and cmax0 evaluated below)
      // Smid = Smid^trial - ga6 Emid,a dg6/dSa - ga0 Emid,a dg0/dSa
      //      = Smid^trial - ga6 cmid6 - ga0 cmid0
      // Smin = Smin^trial - ga6 Emin,a dg6/dSa - ga0 Emin,a dg0/dSa
      //      = Smin^trial - ga6 cmin6 - ga0 cmin0
      const Real cmax6 = _Eij[2][2] * halfplus + _Eij[2][0] * neghalfplus;
      const Real cmax0 = _Eij[2][2];
      const Real cmid6 = _Eij[1][2] * halfplus + _Eij[1][0] * neghalfplus;
      const Real cmid0 = _Eij[1][2];
      const Real cmin6 = _Eij[0][2] * halfplus + _Eij[0][0] * neghalfplus;
      const Real cmin0 = _Eij[0][2];
      // Substituting these into f6 = 0 yields
      // 0 = f6_trial - ga6 (0.5(cmax6 - cmin6) + 0.5(cmax6 + cmin6)sinphi) - ga0 (0.5(cmax0 -
      // cmin0) + 0.5(cmax0 + cmin0)sinphi) = f6_trial - ga6 c6 - ga0 c0, where
      const Real c6 = 0.5 * (cmax6 - cmin6) + 0.5 * (cmax6 + cmin6) * sinphi;
      const Real c0 = 0.5 * (cmax0 - cmin0) + 0.5 * (cmax0 + cmin0) * sinphi;
      // Substituting these into f0 = 0 yields
      // 0 = f0_trial - ga6 cmax6 - ga0 cmax0
      // These equations may be inverted to yield the following
      const Real descr = c0 * cmax6 - c6 * cmax0;
      if (descr != 0.0)
      {
        const Real ga0 = (-c6 * trial_tensile_yf + cmax6 * trial_mc_yf) / descr;
        const Real ga6 = (c0 * trial_tensile_yf - cmax0 * trial_mc_yf) / descr;
        stress_params[2] = trial_stress_params[2] - ga6 * cmax6 - ga0 * cmax0;
        stress_params[1] = trial_stress_params[1] - ga6 * cmid6 - ga0 * cmid0;
        stress_params[0] = trial_stress_params[0] - ga6 * cmin6 - ga0 * cmin0;

        // enforce physicality (go to corners if necessary)
        stress_params[0] =
            std::min(stress_params[0],
                     stress_params[2] - _shifter); // account for poor choice from user
        // if goto_corner then the max(min()) in the subsequent line will force the solution to lie
        // at the corner where max = mid = tensile.  This means the signs of ga0 and ga6 become
        // irrelevant in the check below
        const bool goto_corner = (stress_params[1] >= stress_params[2] - 0.5 * _shifter);
        stress_params[1] = std::max(std::min(stress_params[1], stress_params[2] - 0.5 * _shifter),
                                    stress_params[0] + 0.5 * _shifter);

        const Real new_compressive_yf = -stress_params[0] - cs;
        if (new_compressive_yf <= _f_tol &&
            (goto_corner || (ga0 >= 0.0 && ga6 >= 0.0))) // enforce ga>=0 unless going to a corner
        {
          gaE = ((trial_stress_params[2] - trial_stress_params[0]) -
                 (stress_params[2] - stress_params[0])) /
                    (_Eij[2][2] - _Eij[0][2]) * _Eij[2][2] +
                (trial_stress_params[2] - stress_params[2]);
          found_solution = true;
        }
      }
    }
    if (!found_solution && !mc_impossible)
    {
      // Return to the line where yf[3] = 0 = yf[6].
      // To return to this line, we need to solve f3 = 0 = f6 and
      // the three flow-direction equations.  In the flow-direction equations
      // there are two plasticity multipliers, which i call ga3 and ga6
      // corresponding to the amounts of strain normal to the f3 and f6
      // directions, respectively.
      // So:
      // Smax = Smax^trial - ga6 Emax,a dg6/dSa - ga3 Emax,a dg3/dSa
      //      = Smax^trial - ga6 cmax6 - ga3 cmax3  (with cmax6 and cmax3 evaluated below)
      // Smid = Smid^trial - ga6 Emid,a dg6/dSa - ga3 Emid,a dg3/dSa
      //      = Smid^trial - ga6 cmid6 - ga3 cmid3
      // Smin = Smin^trial - ga6 Emin,a dg6/dSa - ga3 Emin,a dg3/dSa
      //      = Smin^trial - ga6 cmin6 - ga3 cmin3
      const Real cmax6 = _Eij[2][2] * halfplus + _Eij[2][0] * neghalfplus;
      const Real cmax3 = -_Eij[2][0];
      const Real cmid6 = _Eij[1][2] * halfplus + _Eij[1][0] * neghalfplus;
      const Real cmid3 = -_Eij[1][0];
      const Real cmin6 = _Eij[0][2] * halfplus + _Eij[0][0] * neghalfplus;
      const Real cmin3 = -_Eij[0][0];
      // Substituting these into f6 = 0 yields
      // 0 = f6_trial - ga6 (0.5(cmax6 - cmin6) + 0.5(cmax6 + cmin6)sinphi) - ga3 (0.5(cmax3 -
      // cmin3) + 0.5(cmax3 + cmin3)sinphi) = f6_trial - ga6 c6 - ga3 c3, where
      const Real c6 = 0.5 * (cmax6 - cmin6) + 0.5 * (cmax6 + cmin6) * sinphi;
      const Real c3 = 0.5 * (cmax3 - cmin3) + 0.5 * (cmax3 + cmin3) * sinphi;
      // Substituting these into f3 = 0 yields
      // 0 = - f3_trial - ga6 cmin6 - ga3 cmin3
      // These equations may be inverted to yield the following
      const Real descr = c3 * cmin6 - c6 * cmin3;
      if (descr != 0.0)
      {
        const Real ga3 = (c6 * trial_compressive_yf + cmin6 * trial_mc_yf) / descr;
        const Real ga6 = (-c3 * trial_compressive_yf - cmin3 * trial_mc_yf) / descr;
        stress_params[2] = trial_stress_params[2] - ga6 * cmax6 - ga3 * cmax3;
        stress_params[1] = trial_stress_params[1] - ga6 * cmid6 - ga3 * cmid3;
        stress_params[0] = trial_stress_params[0] - ga6 * cmin6 - ga3 * cmin3;

        const Real new_tensile_yf = stress_params[2] - ts;
        stress_params[2] =
            std::max(stress_params[2],
                     stress_params[0] + _shifter); // account for poor choice from user
        stress_params[1] = std::max(std::min(stress_params[1], stress_params[2] - 0.5 * _shifter),
                                    stress_params[0] + 0.5 * _shifter);

        if (new_tensile_yf <= _f_tol && ga6 >= 0.0)
        {
          gaE = ((trial_stress_params[2] - trial_stress_params[0]) -
                 (stress_params[2] - stress_params[0])) /
                    (_Eij[2][2] - _Eij[0][2]) * _Eij[2][2] +
                (-trial_stress_params[0] - stress_params[0]);

          found_solution = true;
        }
      }
    }
    if (!found_solution)
    {
      // Cannot find an acceptable initialisation
      for (unsigned i = 0; i < _num_sp; ++i)
        stress_params[i] = trial_stress_params[i];
      gaE = 0.0;
      mooseWarning("CappedMohrCoulombStressUpdate cannot initialize from max = ",
                   stress_params[2],
                   " mid = ",
                   stress_params[1],
                   " min = ",
                   stress_params[0]);
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
            drot_dstress(i, j, k, l) +=
                0.5 * _eigvecs(i, a) *
                (_eigvecs(k, a) * _eigvecs(l, j) + _eigvecs(l, a) * _eigvecs(k, j)) /
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
