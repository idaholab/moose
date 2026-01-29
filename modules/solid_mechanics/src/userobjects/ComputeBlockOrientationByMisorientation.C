//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeBlockOrientationByMisorientation.h"
#include "MooseMesh.h"

#include "libmesh/mesh_tools.h"

registerMooseObject("SolidMechanicsApp", ComputeBlockOrientationByMisorientation);

InputParameters
ComputeBlockOrientationByMisorientation::validParams()
{
  InputParameters params = ComputeBlockOrientationBase::validParams();
  params.addClassDescription("This object computes the orientation of each grain (block) by "
                             "calculating the maximum misorientation within the grain.");
  return params;
}

ComputeBlockOrientationByMisorientation::ComputeBlockOrientationByMisorientation(
    const InputParameters & parameters)
  : ComputeBlockOrientationBase(parameters),
    _updated_rotation(getMaterialProperty<RankTwoTensor>("updated_rotation")),
    _misorient(getMaterialProperty<Real>("misorientation"))
{
}

void
ComputeBlockOrientationByMisorientation::initialize()
{
  ComputeBlockOrientationBase::initialize();

  _block_ea_values.clear();
  _grain_misorientation.clear();
}

void
ComputeBlockOrientationByMisorientation::execute()
{
  RankTwoTensor rot;
  Real misorient = 0.0;

  for (unsigned int _qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    rot += _JxW[_qp] * _coord[_qp] * _updated_rotation[_qp];
    misorient += _JxW[_qp] * _coord[_qp] * _misorient[_qp];
  }
  rot /= _current_elem_volume;
  misorient /= _current_elem_volume;

  // transform RankTwoTensor to Eigen::Matrix
  Eigen::Matrix<Real, 3, 3> rot_mat;

  for (unsigned int i = 0; i < 3; ++i)
    for (unsigned int j = 0; j < 3; ++j)
      rot_mat(i, j) = rot(i, j);

  // SVD-based projection to SO(3)
  Eigen::JacobiSVD<Eigen::Matrix<Real, 3, 3>> svd(rot_mat,
                                                  Eigen::ComputeFullU | Eigen::ComputeFullV);

  Eigen::Matrix<Real, 3, 3> U = svd.matrixU();
  Eigen::Matrix<Real, 3, 3> V = svd.matrixV();

  // Enforce proper rotation (det = +1)
  Eigen::Matrix<Real, 3, 3> R = U * V.transpose();
  if (R.determinant() < 0.0)
  {
    Eigen::Matrix<Real, 3, 3> D = Eigen::Matrix<Real, 3, 3>::Identity();
    D(2, 2) = -1.0;
    R = U * D * V.transpose();
  }

  // compute Quaternion from rotation matrix
  Eigen::Quaternion<Real> q(R);

  // compute EulerAngle from Quaternion
  EulerAngles ea = EulerAngles(q);

  // store value for the current misorientation in the subdomain
  // save EulerAngle in tuple so that we can gather the data from all processors
  _grain_misorientation[_current_elem->subdomain_id()].emplace_back(
      misorient, ea.phi1, ea.Phi, ea.phi2);
}

void
ComputeBlockOrientationByMisorientation::finalize()
{
  for (const auto & block : _fe_problem.mesh().meshSubdomains())
  {
    _communicator.allgather(_grain_misorientation[block]);
    _block_ea_values[block] = computeSubdomainEulerAngles(block);
  }
}

EulerAngles
ComputeBlockOrientationByMisorientation::computeSubdomainEulerAngles(const SubdomainID & sid)
{
  Real max_misorientation = 0.0; // misorientation values should within [0, pi]
  EulerAngles ea;
  bool has_update = false;
  for (const auto & [misorientation, phi1, Phi, phi2] : _grain_misorientation[sid])
  {
    if (misorientation > max_misorientation)
    {
      max_misorientation = misorientation;
      ea = EulerAngles(phi1, Phi, phi2);
      has_update = true;
    }
  }

  // check if we actually update EulerAngle based on misorientation
  // if not, grab it from the previous step
  if (!has_update)
    return _block_ea_values[sid];

  return ea;
}
