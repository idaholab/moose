//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeUpdatedEulerAngle.h"

registerMooseObject("SolidMechanicsApp", ComputeUpdatedEulerAngle);

InputParameters
ComputeUpdatedEulerAngle::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription(
      "This class computes the updated Euler angle for crystal plasticity simulations. This needs "
      "to be used together with the  ComputeMultipleCrystalPlasticityStress class, where the "
      "updated rotation material property is computed. ");
  params.addParam<bool>(
      "radian_to_degree", true, "Whether to convert euler angles from radian to degree.");
  return params;
}

ComputeUpdatedEulerAngle::ComputeUpdatedEulerAngle(const InputParameters & parameters)
  : Material(parameters),
    _updated_rotation(getMaterialProperty<RankTwoTensor>("updated_rotation")),
    _updated_euler_angle(declareProperty<RealVectorValue>("updated_Euler_angle"))
{
}

void
ComputeUpdatedEulerAngle::initQpStatefulProperties()
{
  _updated_euler_angle[_qp].zero();
}

void
ComputeUpdatedEulerAngle::computeQpProperties()
{
  computeEulerAngleFromRotationMatrix(_updated_rotation[_qp], _updated_euler_angle[_qp]);
}

void
ComputeUpdatedEulerAngle::computeEulerAngleFromRotationMatrix(const RankTwoTensor & rot,
                                                              RealVectorValue & euler_angle)
{
  // transform RankTwoTensor to Eigen::Matrix
  Eigen::Matrix<Real, 3, 3> rot_mat;

  for (unsigned int i = 0; i < 3; ++i)
    for (unsigned int j = 0; j < 3; ++j)
      rot_mat(i, j) = rot(i, j);

  // compute Quaternion from rotation matrix
  Eigen::Quaternion<Real> q(rot_mat);
  // construct Euler angle from Quaternion
  EulerAngles ea(q);
  // convert EulerAngles to RealVectorValue
  euler_angle = (RealVectorValue)ea;

  if (!getParam<bool>("radian_to_degree"))
    euler_angle *= libMesh::pi / 180.0;
}
