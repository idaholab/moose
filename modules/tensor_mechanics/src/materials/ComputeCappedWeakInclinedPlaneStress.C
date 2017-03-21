/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ComputeCappedWeakInclinedPlaneStress.h"
#include "RotationMatrix.h" // for rotVecToZ
#include "libmesh/utility.h"

template <>
InputParameters
validParams<ComputeCappedWeakInclinedPlaneStress>()
{
  InputParameters params = validParams<ComputeCappedWeakPlaneStress>();
  params.addClassDescription("Capped weak inclined plane plasticity stress calculator");
  params.addRequiredParam<RealVectorValue>("normal_vector", "The normal vector to the weak plane");
  return params;
}

ComputeCappedWeakInclinedPlaneStress::ComputeCappedWeakInclinedPlaneStress(
    const InputParameters & parameters)
  : ComputeCappedWeakPlaneStress(parameters),
    _n_input(getParam<RealVectorValue>("normal_vector")),
    _n(declareProperty<RealVectorValue>("weak_plane_normal")),
    _n_old(declarePropertyOld<RealVectorValue>("weak_plane_normal")),
    _rot_n_to_z(RealTensorValue()),
    _rot_z_to_n(RealTensorValue()),
    _rotated_trial(RankTwoTensor()),
    _rotated_Eijkl(RankFourTensor())
{
  if (_n_input.norm() == 0)
    mooseError("ComputeCappedWeakInclinedPlaneStress: normal_vector must not have zero length");
  else
    _n_input /= _n_input.norm();

  _rot_n_to_z = RotationMatrix::rotVecToZ(_n_input);
  _rot_z_to_n = _rot_n_to_z.transpose();
}

void
ComputeCappedWeakInclinedPlaneStress::initQpStatefulProperties()
{
  ComputeCappedWeakPlaneStress::initQpStatefulProperties();
  _n[_qp] = _n_input;
}

void
ComputeCappedWeakInclinedPlaneStress::computeQpStress()
{
  ComputeCappedWeakPlaneStress::computeQpStress();

  if (_perform_finite_strain_rotations)
    for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
    {
      _n[_qp](i) = 0.0;
      for (unsigned int j = 0; j < LIBMESH_DIM; ++j)
        _n[_qp](i) += _rotation_increment[_qp](i, j) * _n_old[_qp](j);
    }
}

void
ComputeCappedWeakInclinedPlaneStress::initialiseReturnProcess()
{
  ComputeCappedWeakPlaneStress::initialiseReturnProcess();
  if (_perform_finite_strain_rotations)
  {
    _rot_n_to_z = RotationMatrix::rotVecToZ(_n[_qp]);
    _rot_z_to_n = _rot_n_to_z.transpose();
  }
}

void
ComputeCappedWeakInclinedPlaneStress::preReturnMap(Real /*p_trial*/,
                                                   Real q_trial,
                                                   const RankTwoTensor & stress_trial,
                                                   const std::vector<Real> & /*intnl_old*/,
                                                   const std::vector<Real> & yf)
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

  _rotated_Eijkl = _elasticity_tensor[_qp];
  _rotated_Eijkl.rotate(_rot_n_to_z);
}

void
ComputeCappedWeakInclinedPlaneStress::computePQ(const RankTwoTensor & stress,
                                                Real & p,
                                                Real & q) const
{
  RankTwoTensor rotated_stress = stress;
  rotated_stress.rotate(_rot_n_to_z);
  p = rotated_stress(2, 2);
  q = std::sqrt(Utility::pow<2>(rotated_stress(0, 2)) + Utility::pow<2>(rotated_stress(1, 2)));
}

void
ComputeCappedWeakInclinedPlaneStress::setEppEqq(const RankFourTensor & /*Eijkl*/,
                                                Real & Epp,
                                                Real & Eqq) const
{
  Epp = _rotated_Eijkl(2, 2, 2, 2);
  Eqq = _rotated_Eijkl(0, 2, 0, 2);
}

void
ComputeCappedWeakInclinedPlaneStress::setStressAfterReturn(const RankTwoTensor & /*stress_trial*/,
                                                           Real p_ok,
                                                           Real q_ok,
                                                           Real gaE,
                                                           const std::vector<Real> & /*intnl*/,
                                                           const f_and_derivs & smoothed_q,
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
ComputeCappedWeakInclinedPlaneStress::consistentTangentOperator(const RankTwoTensor & stress_trial,
                                                                Real p_trial,
                                                                Real q_trial,
                                                                const RankTwoTensor & stress,
                                                                Real p,
                                                                Real q,
                                                                Real gaE,
                                                                const f_and_derivs & smoothed_q,
                                                                RankFourTensor & cto) const
{
  PQPlasticModel::consistentTangentOperator(
      stress_trial, p_trial, q_trial, stress, p, q, gaE, smoothed_q, cto);
}

RankTwoTensor
ComputeCappedWeakInclinedPlaneStress::dpdstress(const RankTwoTensor & /*stress*/) const
{
  RankTwoTensor dpdsig = RankTwoTensor();
  dpdsig(2, 2) = 1.0;
  dpdsig.rotate(_rot_z_to_n);
  return dpdsig;
}

RankTwoTensor
ComputeCappedWeakInclinedPlaneStress::dqdstress(const RankTwoTensor & stress) const
{
  RankTwoTensor rotated_stress = stress;
  rotated_stress.rotate(_rot_n_to_z);
  RankTwoTensor dqdsig = ComputeCappedWeakPlaneStress::dqdstress(rotated_stress);
  dqdsig.rotate(_rot_z_to_n);
  return dqdsig;
}

RankFourTensor
ComputeCappedWeakInclinedPlaneStress::d2qdstress2(const RankTwoTensor & stress) const
{
  RankTwoTensor rotated_stress = stress;
  rotated_stress.rotate(_rot_n_to_z);
  RankFourTensor d2qdsig2 = ComputeCappedWeakPlaneStress::d2qdstress2(rotated_stress);
  d2qdsig2.rotate(_rot_z_to_n);
  return d2qdsig2;
}
