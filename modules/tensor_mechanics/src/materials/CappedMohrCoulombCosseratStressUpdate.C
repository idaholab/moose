//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CappedMohrCoulombCosseratStressUpdate.h"
#include "libmesh/utility.h"

registerMooseObject("TensorMechanicsApp", CappedMohrCoulombCosseratStressUpdate);

InputParameters
CappedMohrCoulombCosseratStressUpdate::validParams()
{
  InputParameters params = CappedMohrCoulombStressUpdate::validParams();
  params.addClassDescription("Capped Mohr-Coulomb plasticity stress calculator for the Cosserat "
                             "situation where the host medium (ie, the limit where all Cosserat "
                             "effects are zero) is isotropic.  Note that the return-map flow rule "
                             "uses an isotropic elasticity tensor built with the 'host' properties "
                             "defined by the user.");
  params.addRequiredRangeCheckedParam<Real>("host_youngs_modulus",
                                            "host_youngs_modulus>0",
                                            "Young's modulus for the isotropic host medium");
  params.addRequiredRangeCheckedParam<Real>("host_poissons_ratio",
                                            "host_poissons_ratio>=0 & host_poissons_ratio<0.5",
                                            "Poisson's ratio for the isotropic host medium");
  return params;
}

CappedMohrCoulombCosseratStressUpdate::CappedMohrCoulombCosseratStressUpdate(
    const InputParameters & parameters)
  : CappedMohrCoulombStressUpdate(parameters),
    _host_young(getParam<Real>("host_youngs_modulus")),
    _host_poisson(getParam<Real>("host_poissons_ratio")),
    _host_E0011(_host_young * _host_poisson / (1.0 + _host_poisson) / (1.0 - 2.0 * _host_poisson)),
    _host_E0000(_host_E0011 + _host_young / (1.0 + _host_poisson))
{
}

void
CappedMohrCoulombCosseratStressUpdate::preReturnMapV(
    const std::vector<Real> & /*trial_stress_params*/,
    const RankTwoTensor & stress_trial,
    const std::vector<Real> & /*intnl_old*/,
    const std::vector<Real> & /*yf*/,
    const RankFourTensor & /*Eijkl*/)
{
  std::vector<Real> eigvals;
  stress_trial.symmetricEigenvaluesEigenvectors(eigvals, _eigvecs);
  _poissons_ratio = _host_poisson;
}

void
CappedMohrCoulombCosseratStressUpdate::setEffectiveElasticity(const RankFourTensor & /*Eijkl*/)
{
  _Eij[0][0] = _Eij[1][1] = _Eij[2][2] = _host_E0000;
  _Eij[0][1] = _Eij[1][0] = _Eij[0][2] = _Eij[2][0] = _Eij[1][2] = _Eij[2][1] = _host_E0011;
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
CappedMohrCoulombCosseratStressUpdate::setStressAfterReturnV(
    const RankTwoTensor & stress_trial,
    const std::vector<Real> & stress_params,
    Real /*gaE*/,
    const std::vector<Real> & /*intnl*/,
    const yieldAndFlow & /*smoothed_q*/,
    const RankFourTensor & /*Eijkl*/,
    RankTwoTensor & stress) const
{
  // form the diagonal stress
  stress = RankTwoTensor(stress_params[0], stress_params[1], stress_params[2], 0.0, 0.0, 0.0);
  // rotate to the original frame, to give the symmetric part of the stress
  stress = _eigvecs * stress * (_eigvecs.transpose());
  // add the non-symmetric parts
  stress += 0.5 * (stress_trial - stress_trial.transpose());
}

void
CappedMohrCoulombCosseratStressUpdate::consistentTangentOperatorV(
    const RankTwoTensor & stress_trial,
    const std::vector<Real> & trial_stress_params,
    const RankTwoTensor & stress,
    const std::vector<Real> & stress_params,
    Real gaE,
    const yieldAndFlow & smoothed_q,
    const RankFourTensor & elasticity_tensor,
    bool compute_full_tangent_operator,
    const std::vector<std::vector<Real>> & dvar_dtrial,
    RankFourTensor & cto)
{
  CappedMohrCoulombStressUpdate::consistentTangentOperatorV(stress_trial,
                                                            trial_stress_params,
                                                            stress,
                                                            stress_params,
                                                            gaE,
                                                            smoothed_q,
                                                            elasticity_tensor,
                                                            compute_full_tangent_operator,
                                                            dvar_dtrial,
                                                            cto);

  if (!compute_full_tangent_operator)
    return;

  /**
   * Add the correction for the antisymmetric part of the elasticity
   * tensor.
   * CappedMohrCoulombStressUpdate computes
   * cto(i, j, k, l) = dstress(i, j)/dstrain(k, l)
   * and during the computations it explicitly performs certain
   * contractions that result in symmetry between i and j, and k and l,
   * viz, cto(i, j, k, l) = cto(j, i, k, l) = cto(i, j, l, k)
   * That is correct because that plasticity model is only valid for
   * symmetric stresses and strains.
   * CappedMohrCoulombCosseratStressUpdate does not include contributions from the
   * antisymmetric parts of stress (or strain), so the antisymmetric
   * parts of cto are just the antisymmetric parts of the elasticity
   * tensor, which must now get added to the cto computed by
   * CappedMohrCoulombStressUpdate
   */
  RankFourTensor anti;
  for (unsigned i = 0; i < _tensor_dimensionality; ++i)
    for (unsigned j = 0; j < _tensor_dimensionality; ++j)
      for (unsigned k = 0; k < _tensor_dimensionality; ++k)
        for (unsigned l = 0; l < _tensor_dimensionality; ++l)
          anti(i, j, k, l) = 0.5 * (elasticity_tensor(i, j, k, l) - elasticity_tensor(j, i, k, l));

  cto += anti;
}
