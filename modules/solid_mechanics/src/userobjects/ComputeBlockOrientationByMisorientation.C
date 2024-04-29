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
  _block_ea_values.clear();
  _grain_misorientation.clear();
}

void
ComputeBlockOrientationByMisorientation::execute()
{
  // Compute the average of the rotation matrix and misorientation in this element
  RankTwoTensor rot;
  MathUtils::mooseSetToZero(rot);
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

  // compute Quaternion from rotation matrix
  Eigen::Quaternion<Real> q(rot_mat);

  // compute EulerAngle from Quaternion
  EulerAngles ea = EulerAngles(q);

  // store value for the current misorientation in the subdomain
  // save EulerAngle in tuple so that we can gather the data from all processors
  _grain_misorientation[_current_elem->subdomain_id()].push_back(
      std::make_tuple(misorient, ea.phi1, ea.Phi, ea.phi2));
}

void
ComputeBlockOrientationByMisorientation::threadJoin(const UserObject & y)
{
  ElementUserObject::threadJoin(y);

  // We are joining with another class like this one so do a cast so we can get to it's data
  const ComputeBlockOrientationByMisorientation & cbo =
      dynamic_cast<const ComputeBlockOrientationByMisorientation &>(y);

  for (auto it = cbo._grain_misorientation.begin(); it != cbo._grain_misorientation.end(); ++it)
    _grain_misorientation[it->first].insert(
        _grain_misorientation[it->first].end(), it->second.begin(), it->second.end());
}

void
ComputeBlockOrientationByMisorientation::finalize()
{
  const std::set<SubdomainID> & blocks = _fe_problem.mesh().meshSubdomains();

  for (std::set<SubdomainID>::const_iterator it = blocks.begin(); it != blocks.end(); ++it)
  {
    // Sync data from all processors (gather the maximum misorientation and the corresponding
    // EulerAngle from every processor)
    _communicator.allgather(_grain_misorientation[*it]);
    _block_ea_values[*it] = computeSubdomainEulerAngles(*it);
  }
}

EulerAngles
ComputeBlockOrientationByMisorientation::computeSubdomainEulerAngles(const SubdomainID & sid)
{
  Real max_misorientation = 0.0; // misorientation values should within [0, pi]
  EulerAngles ea;
  bool has_update = false;
  for (const auto & orient : _grain_misorientation[sid])
  {
    if (std::get<0>(orient) > max_misorientation)
    {
      max_misorientation = std::get<0>(orient);
      ea = EulerAngles(std::get<1>(orient), std::get<2>(orient), std::get<3>(orient));
      has_update = true;
    }
  }

  // check if we actually update EulerAngle based on misorientation
  // if not, grab it from the previous step
  if (!has_update)
    return _block_ea_values[sid];

  return ea;
}
