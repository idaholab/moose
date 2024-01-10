//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CappedWeakInclinedPlaneStressUpdate.h"
#include "RotationMatrix.h" // for rotVecToZ
#include "libmesh/utility.h"

registerMooseObject("TensorMechanicsApp", CappedWeakInclinedPlaneStressUpdate);

InputParameters
CappedWeakInclinedPlaneStressUpdate::validParams()
{
  InputParameters params = CappedWeakPlaneStressUpdate::validParams();
  params.addClassDescription("Capped weak inclined plane plasticity stress calculator");
  params.addRequiredParam<RealVectorValue>("normal_vector", "The normal vector to the weak plane");
  return params;
}

CappedWeakInclinedPlaneStressUpdate::CappedWeakInclinedPlaneStressUpdate(
    const InputParameters & parameters)
  : CappedWeakPlaneStressUpdate(parameters),
    _n_input(getParam<RealVectorValue>("normal_vector")),
    _n(declareProperty<RealVectorValue>("weak_plane_normal")),
    _n_old(getMaterialProperty<RealVectorValue>("weak_plane_normal")),
    _rot_n_to_z(RealTensorValue()),
    _rot_z_to_n(RealTensorValue()),
    _rotated_trial(RankTwoTensor()),
    _rotated_Eijkl(RankFourTensor())
{
  if (_n_input.norm() == 0)
    mooseError("CappedWeakInclinedPlaneStressUpdate: normal_vector must not have zero length");
  else
    _n_input /= _n_input.norm();

  _rot_n_to_z = RotationMatrix::rotVecToZ(_n_input);
  _rot_z_to_n = _rot_n_to_z.transpose();
}

void
CappedWeakInclinedPlaneStressUpdate::initQpStatefulProperties()
{
  CappedWeakPlaneStressUpdate::initQpStatefulProperties();
  _n[_qp] = _n_input;
}

void
CappedWeakInclinedPlaneStressUpdate::finalizeReturnProcess(const RankTwoTensor & rotation_increment)
{
  CappedWeakPlaneStressUpdate::finalizeReturnProcess(rotation_increment);
  if (_perform_finite_strain_rotations)
    for (const auto i : make_range(Moose::dim))
    {
      _n[_qp](i) = 0.0;
      for (const auto j : make_range(Moose::dim))
        _n[_qp](i) += rotation_increment(i, j) * _n_old[_qp](j);
    }
}

void
CappedWeakInclinedPlaneStressUpdate::initializeReturnProcess()
{
  CappedWeakPlaneStressUpdate::initializeReturnProcess();
  if (_perform_finite_strain_rotations)
  {
    _rot_n_to_z = RotationMatrix::rotVecToZ(_n[_qp]);
    _rot_z_to_n = _rot_n_to_z.transpose();
  }
}

void
CappedWeakInclinedPlaneStressUpdate::preReturnMap(Real /*p_trial*/,
                                                  Real q_trial,
                                                  const RankTwoTensor & stress_trial,
                                                  const std::vector<Real> & /*intnl_old*/,
                                                  const std::vector<Real> & yf,
                                                  const RankFourTensor & Eijkl)
{
  // If it's obvious, then simplify the return-type
  if (yf[1] >= 0)
    _stress_return_type = StressReturnType::no_compression;
  else if (yf[2] >= 0)
    _stress_return_type = StressReturnType::no_tension;

  _rotated_trial = stress_trial;
  _rotated_trial.rotate(_rot_n_to_z);
  _in_trial02 = _rotated_trial(0, 2);
  _in_trial12 = _rotated_trial(1, 2);
  _in_q_trial = q_trial;

  _rotated_Eijkl = Eijkl;
  _rotated_Eijkl.rotate(_rot_n_to_z);
}

void
CappedWeakInclinedPlaneStressUpdate::computePQ(const RankTwoTensor & stress,
                                               Real & p,
                                               Real & q) const
{
  RankTwoTensor rotated_stress = stress;
  rotated_stress.rotate(_rot_n_to_z);
  p = rotated_stress(2, 2);
  q = std::sqrt(Utility::pow<2>(rotated_stress(0, 2)) + Utility::pow<2>(rotated_stress(1, 2)));
}

void
CappedWeakInclinedPlaneStressUpdate::setEppEqq(const RankFourTensor & /*Eijkl*/,
                                               Real & Epp,
                                               Real & Eqq) const
{
  Epp = _rotated_Eijkl(2, 2, 2, 2);
  Eqq = _rotated_Eijkl(0, 2, 0, 2);
}

void
CappedWeakInclinedPlaneStressUpdate::setStressAfterReturn(const RankTwoTensor & /*stress_trial*/,
                                                          Real p_ok,
                                                          Real q_ok,
                                                          Real gaE,
                                                          const std::vector<Real> & /*intnl*/,
                                                          const yieldAndFlow & smoothed_q,
                                                          const RankFourTensor & /*Eijkl*/,
                                                          RankTwoTensor & stress) const
{
  // first get stress in the frame where _n points along "z"
  stress = _rotated_trial;
  stress(2, 2) = p_ok;
  // stress_xx and stress_yy are sitting at their trial-stress values
  // so need to bring them back via Poisson's ratio
  stress(0, 0) -= _rotated_Eijkl(2, 2, 0, 0) * gaE / _Epp * smoothed_q.dg[0];
  stress(1, 1) -= _rotated_Eijkl(2, 2, 1, 1) * gaE / _Epp * smoothed_q.dg[0];
  if (_in_q_trial == 0.0)
    stress(2, 0) = stress(2, 1) = stress(0, 2) = stress(1, 2) = 0.0;
  else
  {
    stress(2, 0) = stress(0, 2) = _in_trial02 * q_ok / _in_q_trial;
    stress(2, 1) = stress(1, 2) = _in_trial12 * q_ok / _in_q_trial;
  }

  // rotate back to the original frame
  stress.rotate(_rot_z_to_n);
}

void
CappedWeakInclinedPlaneStressUpdate::consistentTangentOperator(const RankTwoTensor & stress_trial,
                                                               Real p_trial,
                                                               Real q_trial,
                                                               const RankTwoTensor & stress,
                                                               Real p,
                                                               Real q,
                                                               Real gaE,
                                                               const yieldAndFlow & smoothed_q,
                                                               const RankFourTensor & Eijkl,
                                                               bool compute_full_tangent_operator,
                                                               RankFourTensor & cto) const
{
  TwoParameterPlasticityStressUpdate::consistentTangentOperator(stress_trial,
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

RankTwoTensor
CappedWeakInclinedPlaneStressUpdate::dpdstress(const RankTwoTensor & /*stress*/) const
{
  RankTwoTensor dpdsig = RankTwoTensor();
  dpdsig(2, 2) = 1.0;
  dpdsig.rotate(_rot_z_to_n);
  return dpdsig;
}

RankTwoTensor
CappedWeakInclinedPlaneStressUpdate::dqdstress(const RankTwoTensor & stress) const
{
  RankTwoTensor rotated_stress = stress;
  rotated_stress.rotate(_rot_n_to_z);
  RankTwoTensor dqdsig = CappedWeakPlaneStressUpdate::dqdstress(rotated_stress);
  dqdsig.rotate(_rot_z_to_n);
  return dqdsig;
}

RankFourTensor
CappedWeakInclinedPlaneStressUpdate::d2qdstress2(const RankTwoTensor & stress) const
{
  RankTwoTensor rotated_stress = stress;
  rotated_stress.rotate(_rot_n_to_z);
  RankFourTensor d2qdsig2 = CappedWeakPlaneStressUpdate::d2qdstress2(rotated_stress);
  d2qdsig2.rotate(_rot_z_to_n);
  return d2qdsig2;
}
