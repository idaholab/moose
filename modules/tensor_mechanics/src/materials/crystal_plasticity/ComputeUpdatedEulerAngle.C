//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeUpdatedEulerAngle.h"

registerMooseObject("TensorMechanicsApp", ComputeUpdatedEulerAngle);

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
  Real phi1, Phi, phi2;

  // contert to Bunge's Euler angle
  Phi = std::acos(rot(2, 2));
  if (MooseUtils::absoluteFuzzyEqual(std::abs(rot(2, 2)) - 1.0, 0.0))
  {
    phi1 = 0.0;
    phi2 = std::atan2(rot(0, 1), rot(0, 0));
  }
  else
  {
    Real sPhi = std::sin(Phi);
    phi1 = std::atan2(rot(0, 2) / sPhi, rot(1, 2) / sPhi);
    phi2 = std::atan2(rot(2, 0) / sPhi, -rot(2, 1) / sPhi);
  }

  euler_angle(0) = phi1;
  euler_angle(1) = Phi;
  euler_angle(2) = phi2;

  if (getParam<bool>("radian_to_degree"))
    euler_angle *= 180.0 / pi;
}
