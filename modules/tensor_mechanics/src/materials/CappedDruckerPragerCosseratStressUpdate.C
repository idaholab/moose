//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CappedDruckerPragerCosseratStressUpdate.h"

registerMooseObject("TensorMechanicsApp", CappedDruckerPragerCosseratStressUpdate);

InputParameters
CappedDruckerPragerCosseratStressUpdate::validParams()
{
  InputParameters params = CappedDruckerPragerStressUpdate::validParams();
  params.addClassDescription("Capped Drucker-Prager plasticity stress calculator for the Cosserat "
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

CappedDruckerPragerCosseratStressUpdate::CappedDruckerPragerCosseratStressUpdate(
    const InputParameters & parameters)
  : CappedDruckerPragerStressUpdate(parameters),
    _shear(getParam<Real>("host_youngs_modulus") /
           (2.0 * (1.0 + getParam<Real>("host_poissons_ratio"))))
{
  const Real young = getParam<Real>("host_youngs_modulus");
  const Real poisson = getParam<Real>("host_poissons_ratio");
  const Real lambda = young * poisson / ((1.0 + poisson) * (1.0 - 2.0 * poisson));
  _Ehost.fillFromInputVector({lambda, _shear}, RankFourTensor::symmetric_isotropic);
}

void
CappedDruckerPragerCosseratStressUpdate::setEppEqq(const RankFourTensor & /*Eijkl*/,
                                                   Real & Epp,
                                                   Real & Eqq) const
{
  Epp = _Ehost.sum3x3();
  Eqq = _shear;
}

void
CappedDruckerPragerCosseratStressUpdate::setStressAfterReturn(const RankTwoTensor & stress_trial,
                                                              Real p_ok,
                                                              Real q_ok,
                                                              Real /*gaE*/,
                                                              const std::vector<Real> & /*intnl*/,
                                                              const yieldAndFlow & /*smoothed_q*/,
                                                              const RankFourTensor & /*Eijkl*/,
                                                              RankTwoTensor & stress) const
{
  // symm_stress is the symmetric part of the stress tensor.
  // symm_stress = (s_ij+s_ji)/2 + de_ij tr(stress) / 3
  //             = q / q_trial * (s_ij^trial+s_ji^trial)/2 + de_ij p / 3
  //             = q / q_trial * (symm_stress_ij^trial - de_ij tr(stress^trial) / 3) + de_ij p / 3
  const Real p_trial = stress_trial.trace();
  RankTwoTensor symm_stress = RankTwoTensor(RankTwoTensor::initIdentity) / 3.0 *
                              (p_ok - (_in_q_trial == 0.0 ? 0.0 : p_trial * q_ok / _in_q_trial));
  if (_in_q_trial > 0)
    symm_stress += q_ok / _in_q_trial * 0.5 * (stress_trial + stress_trial.transpose());
  stress = symm_stress + 0.5 * (stress_trial - stress_trial.transpose());
}

void
CappedDruckerPragerCosseratStressUpdate::consistentTangentOperator(
    const RankTwoTensor & /*stress_trial*/,
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
  if (!compute_full_tangent_operator)
  {
    cto = Eijkl;
    return;
  }

  RankFourTensor EAijkl;
  for (unsigned i = 0; i < _tensor_dimensionality; ++i)
    for (unsigned j = 0; j < _tensor_dimensionality; ++j)
      for (unsigned k = 0; k < _tensor_dimensionality; ++k)
        for (unsigned l = 0; l < _tensor_dimensionality; ++l)
        {
          cto(i, j, k, l) = 0.5 * (Eijkl(i, j, k, l) + Eijkl(j, i, k, l));
          EAijkl(i, j, k, l) = 0.5 * (Eijkl(i, j, k, l) - Eijkl(j, i, k, l));
        }

  const RankTwoTensor s_over_q =
      (q == 0.0 ? RankTwoTensor()
                : (0.5 * (stress + stress.transpose()) -
                   stress.trace() * RankTwoTensor(RankTwoTensor::initIdentity) / 3.0) /
                      q);
  const RankTwoTensor E_s_over_q = Eijkl.innerProductTranspose(s_over_q); // not symmetric in kl
  const RankTwoTensor Ekl =
      RankTwoTensor(RankTwoTensor::initIdentity).initialContraction(Eijkl); // symmetric in kl

  for (unsigned i = 0; i < _tensor_dimensionality; ++i)
    for (unsigned j = 0; j < _tensor_dimensionality; ++j)
      for (unsigned k = 0; k < _tensor_dimensionality; ++k)
        for (unsigned l = 0; l < _tensor_dimensionality; ++l)
        {
          cto(i, j, k, l) -= (i == j) * (1.0 / 3.0) *
                             (Ekl(k, l) * (1.0 - _dp_dpt) + 0.5 * E_s_over_q(k, l) * (-_dp_dqt));
          cto(i, j, k, l) -=
              s_over_q(i, j) * (Ekl(k, l) * (-_dq_dpt) + 0.5 * E_s_over_q(k, l) * (1.0 - _dq_dqt));
        }

  if (smoothed_q.dg[1] != 0.0)
  {
    const RankFourTensor Tijab = _Ehost * (gaE / _Epp) * smoothed_q.dg[1] * d2qdstress2(stress);
    RankFourTensor inv = RankFourTensor(RankFourTensor::initIdentitySymmetricFour) + Tijab;
    try
    {
      inv = inv.transposeMajor().invSymm();
    }
    catch (const MooseException & e)
    {
      // Cannot form the inverse, so probably at some degenerate place in stress space.
      // Just return with the "best estimate" of the cto.
      mooseWarning("CappedDruckerPragerCosseratStressUpdate: Cannot invert 1+T in consistent "
                   "tangent operator computation at quadpoint ",
                   _qp,
                   " of element ",
                   _current_elem->id());
      return;
    }
    cto = (cto.transposeMajor() * inv).transposeMajor();
  }
  cto += EAijkl;
}
