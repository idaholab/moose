//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TensileStressUpdate.h"
#include "libmesh/utility.h"

registerMooseObject("TensorMechanicsApp", TensileStressUpdate);

InputParameters
TensileStressUpdate::validParams()
{
  InputParameters params = MultiParameterPlasticityStressUpdate::validParams();
  params.addRequiredParam<UserObjectName>(
      "tensile_strength",
      "A TensorMechanicsHardening UserObject that defines hardening of the tensile strength");
  params.addParam<bool>("perfect_guess",
                        true,
                        "Provide a guess to the Newton-Raphson procedure "
                        "that is the result from perfect plasticity.  With "
                        "severe hardening/softening this may be "
                        "suboptimal.");
  params.addClassDescription(
      "Associative, smoothed, tensile (Rankine) plasticity with hardening/softening");
  return params;
}

TensileStressUpdate::TensileStressUpdate(const InputParameters & parameters)
  : MultiParameterPlasticityStressUpdate(parameters, 3, 3, 1),
    _strength(getUserObject<TensorMechanicsHardeningModel>("tensile_strength")),
    _perfect_guess(getParam<bool>("perfect_guess")),
    _eigvecs(RankTwoTensor())
{
}

void
TensileStressUpdate::computeStressParams(const RankTwoTensor & stress,
                                         std::vector<Real> & stress_params) const
{
  // stress_params[0] = smallest eigenvalue, stress_params[2] = largest eigenvalue
  stress.symmetricEigenvalues(stress_params);
}

std::vector<RankTwoTensor>
TensileStressUpdate::dstress_param_dstress(const RankTwoTensor & stress) const
{
  std::vector<Real> sp;
  std::vector<RankTwoTensor> dsp;
  stress.dsymmetricEigenvalues(sp, dsp);
  return dsp;
}

std::vector<RankFourTensor>
TensileStressUpdate::d2stress_param_dstress(const RankTwoTensor & stress) const
{
  std::vector<RankFourTensor> d2;
  stress.d2symmetricEigenvalues(d2);
  return d2;
}

void
TensileStressUpdate::preReturnMapV(const std::vector<Real> & /*trial_stress_params*/,
                                   const RankTwoTensor & stress_trial,
                                   const std::vector<Real> & /*intnl_old*/,
                                   const std::vector<Real> & /*yf*/,
                                   const RankFourTensor & /*Eijkl*/)
{
  std::vector<Real> eigvals;
  stress_trial.symmetricEigenvaluesEigenvectors(eigvals, _eigvecs);
}

void
TensileStressUpdate::setStressAfterReturnV(const RankTwoTensor & /*stress_trial*/,
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
TensileStressUpdate::yieldFunctionValuesV(const std::vector<Real> & stress_params,
                                          const std::vector<Real> & intnl,
                                          std::vector<Real> & yf) const
{
  const Real ts = tensile_strength(intnl[0]);
  // The smoothing strategy means that the last yield function
  // gives the smallest value when returning to a line/point where,
  // without smoothing, the yield functions are equal.
  // Therefore, i use the smallest eigenvalue in this yield function
  yf[0] = stress_params[2] - ts; // use largest eigenvalue
  yf[1] = stress_params[1] - ts;
  yf[2] = stress_params[0] - ts; // use smallest eigenvalue
}

void
TensileStressUpdate::computeAllQV(const std::vector<Real> & stress_params,
                                  const std::vector<Real> & intnl,
                                  std::vector<yieldAndFlow> & all_q) const
{
  const Real ts = tensile_strength(intnl[0]);
  const Real dts = dtensile_strength(intnl[0]);

  // yield functions.  See comment in yieldFunctionValuesV
  all_q[0].f = stress_params[2] - ts;
  all_q[1].f = stress_params[1] - ts;
  all_q[2].f = stress_params[0] - ts;

  // d(yield function)/d(stress_params)
  for (unsigned yf = 0; yf < _num_yf; ++yf)
    for (unsigned a = 0; a < _num_sp; ++a)
      all_q[yf].df[a] = 0.0;
  all_q[0].df[2] = 1.0;
  all_q[1].df[1] = 1.0;
  all_q[2].df[0] = 1.0;

  // d(yield function)/d(intnl)
  for (unsigned yf = 0; yf < _num_yf; ++yf)
    all_q[yf].df_di[0] = -dts;

  // the flow potential is just the yield function
  // d(flow potential)/d(stress_params)
  for (unsigned yf = 0; yf < _num_yf; ++yf)
    for (unsigned a = 0; a < _num_sp; ++a)
      all_q[yf].dg[a] = all_q[yf].df[a];

  // d(flow potential)/d(stress_params)/d(intnl)
  for (unsigned yf = 0; yf < _num_yf; ++yf)
    for (unsigned a = 0; a < _num_sp; ++a)
      all_q[yf].d2g_di[a][0] = 0.0;

  // d(flow potential)/d(stress_params)/d(stress_params)
  for (unsigned yf = 0; yf < _num_yf; ++yf)
    for (unsigned a = 0; a < _num_sp; ++a)
      for (unsigned b = 0; b < _num_sp; ++b)
        all_q[yf].d2g[a][b] = 0.0;
}

void
TensileStressUpdate::setEffectiveElasticity(const RankFourTensor & Eijkl)
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
TensileStressUpdate::initializeVarsV(const std::vector<Real> & trial_stress_params,
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
    const Real ts = tensile_strength(intnl_old[0]);
    stress_params[2] = ts; // largest eigenvalue
    stress_params[1] = std::min(stress_params[1], ts);
    stress_params[0] = std::min(stress_params[0], ts);
    gaE = trial_stress_params[2] - stress_params[2];
  }
  setIntnlValuesV(trial_stress_params, stress_params, intnl_old, intnl);
}

void
TensileStressUpdate::setIntnlValuesV(const std::vector<Real> & trial_stress_params,
                                     const std::vector<Real> & current_stress_params,
                                     const std::vector<Real> & intnl_old,
                                     std::vector<Real> & intnl) const
{
  intnl[0] = intnl_old[0] + (trial_stress_params[2] - current_stress_params[2]) / _Eij[2][2];
}

void
TensileStressUpdate::setIntnlDerivativesV(const std::vector<Real> & /*trial_stress_params*/,
                                          const std::vector<Real> & /*current_stress_params*/,
                                          const std::vector<Real> & /*intnl*/,
                                          std::vector<std::vector<Real>> & dintnl) const
{
  dintnl[0][0] = 0.0;
  dintnl[0][1] = 0.0;
  dintnl[0][2] = -1.0 / _Eij[2][2];
}

Real
TensileStressUpdate::tensile_strength(const Real internal_param) const
{
  return _strength.value(internal_param);
}

Real
TensileStressUpdate::dtensile_strength(const Real internal_param) const
{
  return _strength.derivative(internal_param);
}

void
TensileStressUpdate::consistentTangentOperatorV(const RankTwoTensor & stress_trial,
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
