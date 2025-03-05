//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeBlockOrientationByRotation.h"
#include "MooseMesh.h"

#include "libmesh/mesh_tools.h"

registerMooseObject("SolidMechanicsApp", ComputeBlockOrientationByRotation);

InputParameters
ComputeBlockOrientationByRotation::validParams()
{
  InputParameters params = ComputeBlockOrientationBase::validParams();
  params.addParam<unsigned int>("bins", 20, "Number of bins to segregate quaternions");
  params.addParam<Real>("L_norm", 1, "Specifies the type of average the user intends to perform");
  params.addClassDescription(
      "This object determines the orientation of each grain (block) by identifying the most common "
      "orientation direction among the material points within the grain.");
  return params;
}

ComputeBlockOrientationByRotation::ComputeBlockOrientationByRotation(
    const InputParameters & parameters)
  : ComputeBlockOrientationBase(parameters),
    _updated_rotation(getMaterialProperty<RankTwoTensor>("updated_rotation")),
    _bins(getParam<unsigned int>("bins")),
    _L_norm(getParam<Real>("L_norm"))
{
}

void
ComputeBlockOrientationByRotation::initialize()
{
  ComputeBlockOrientationBase::initialize();

  _block_ea_values.clear();
  _quat.clear();
}

void
ComputeBlockOrientationByRotation::execute()
{
  // Compute the average of the rotation matrix in this element
  RankTwoTensor rot;
  MathUtils::mooseSetToZero(rot);

  for (unsigned int _qp = 0; _qp < _qrule->n_points(); _qp++)
    rot += _JxW[_qp] * _coord[_qp] * _updated_rotation[_qp];
  rot /= _current_elem_volume;

  // transform RankTwoTensor to Eigen::Matrix
  Eigen::Matrix<Real, 3, 3> rot_mat;

  for (unsigned int i = 0; i < 3; ++i)
    for (unsigned int j = 0; j < 3; ++j)
      rot_mat(i, j) = rot(i, j);

  // compute Quaternion from rotation matrix
  Eigen::Quaternion<Real> q(rot_mat);
  // store quaternion values for the current element
  _quat[_current_elem->subdomain_id()].push_back(std::make_tuple(q.w(), q.x(), q.y(), q.z()));
}

void
ComputeBlockOrientationByRotation::finalize()
{
  const std::set<SubdomainID> & blocks = _fe_problem.mesh().meshSubdomains();

  for (std::set<SubdomainID>::const_iterator it = blocks.begin(); it != blocks.end(); ++it)
  {
    // Use allgather to sync data from all processors
    _communicator.allgather(_quat[*it]);
    _block_ea_values[*it] = computeSubdomainEulerAngles(*it);
  }
}

EulerAngles
ComputeBlockOrientationByRotation::computeSubdomainEulerAngles(const SubdomainID & sid)
{
  // creating a map to store the quaternion count for each bin index
  std::map<std::tuple<int, int, int, int>, unsigned int> feature_weights;

  for (const auto & q : _quat[sid])
  {
    const auto bin = std::make_tuple<int, int, int, int>(std::floor(std::get<0>(q) * 0.5 * _bins),
                                                         std::floor(std::get<1>(q) * 0.5 * _bins),
                                                         std::floor(std::get<2>(q) * 0.5 * _bins),
                                                         std::floor(std::get<3>(q) * 0.5 * _bins));
    feature_weights[bin]++;
  }

  /**
   * Markley, F. Landis, Yang Cheng, John Lucas Crassidis, and Yaakov Oshman.
   * "Averaging quaternions." Journal of Guidance, Control, and Dynamics 30,
   * no. 4 (2007): 1193-1197.
   * A 4 by N matrix (Q) is constructed, where N is the number of quaternions.
   * A weight matrix (W) is created. The eigenvector corresponding to the
   * maximum eigenvalue of Q*W*Q' is the weighted average quaternion
   */

  // quaternion average matrix Q*w*Q^T
  typedef Eigen::Matrix<Real, 4, 4> Matrix4x4;
  Matrix4x4 quat_mat = Matrix4x4::Zero();
  typedef Eigen::Matrix<Real, 4, 1> Vector4;

  Real total_weight = 0.0;

  for (const auto & q : _quat[sid])
  {
    Vector4 v(std::get<0>(q), std::get<1>(q), std::get<2>(q), std::get<3>(q));

    const auto bin = std::make_tuple<int, int, int, int>(std::floor(std::get<0>(q) * 0.5 * _bins),
                                                         std::floor(std::get<1>(q) * 0.5 * _bins),
                                                         std::floor(std::get<2>(q) * 0.5 * _bins),
                                                         std::floor(std::get<3>(q) * 0.5 * _bins));
    const auto bin_size = feature_weights[bin];
    const auto weight = std::pow(bin_size, _L_norm);
    total_weight += weight;

    quat_mat += v * weight * v.transpose();
  }

  quat_mat *= 1.0 / total_weight;

  // compute eigenvalues and eigenvectors
  Eigen::EigenSolver<Matrix4x4> EigenSolver(quat_mat);
  Vector4 eigen_values = EigenSolver.eigenvalues().real();
  Matrix4x4 eigen_vectors = EigenSolver.eigenvectors().real();

  // Selecting eigenvector corresponding to max eigenvalue to compute average Euler angle
  Vector4::Index max_index = 0;
  eigen_values.maxCoeff(&max_index);
  const auto max_vec = eigen_vectors.col(max_index) / eigen_vectors.col(max_index).norm();
  const Eigen::Quaternion<Real> q(max_vec(0), max_vec(1), max_vec(2), max_vec(3));

  return EulerAngles(q);
}
