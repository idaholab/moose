//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Assembly.h"
#include "CZMMaterialBase.h"
#include "RotationMatrix.h"

template <>
InputParameters
validParams<CZMMaterialBase>()
{
  InputParameters params = validParams<InterfaceMaterial>();

  params.addParam<Real>("inner_penetration_penalty", 1, "inner penalty factor");
  params.addClassDescription("this material class is used when defining a "
                             "cohesive zone model");
  params.addRequiredCoupledVar("disp_x",
                               "the name of the displacement variable along the x direction");
  params.addRequiredCoupledVar("disp_y",
                               "the name of the displacement variable along the x direction");
  params.addRequiredCoupledVar("disp_z",
                               "the name of the displacement variable along the x direction");
  return params;
}

CZMMaterialBase::CZMMaterialBase(const InputParameters & parameters)
  : InterfaceMaterial(parameters),
    _normals(_assembly.normals()),
    _disp_x(coupledValue("disp_x")),
    _disp_x_neigh(coupledNeighborValue("disp_x")),
    _disp_y(coupledValue("disp_y")),
    _disp_y_neigh(coupledNeighborValue("disp_y")),
    _disp_z(coupledValue("disp_z")),
    _disp_z_neigh(coupledNeighborValue("disp_z")),
    _disp_x_dot(coupledDot("disp_x")),
    _disp_x_neigh_dot(coupledNeighborValueDot("disp_x")),
    _disp_y_dot(coupledDot("disp_y")),
    _disp_y_neigh_dot(coupledNeighborValueDot("disp_y")),
    _disp_z_dot(coupledDot("disp_z")),
    _disp_z_neigh_dot(coupledNeighborValueDot("disp_z")),
    _displacement_jump(declareProperty<RealVectorValue>("displacement_jump")),
    _displacement_jump_local(declareProperty<RealVectorValue>("displacement_jump_local")),
    _displacement_jump_dot(declareProperty<RealVectorValue>("displacement_jump_dot")),
    _displacement_jump_dot_local(declareProperty<RealVectorValue>("displacement_jump_dot_local")),
    _displacement_jump_old(getMaterialPropertyOld<RealVectorValue>("displacement_jump")),
    _displacement_jump_local_old(
        getMaterialPropertyOld<RealVectorValue>("displacement_jump_local")),
    _traction(declareProperty<RealVectorValue>("traction")),
    _traction_local(declareProperty<RealVectorValue>("traction_local")),
    _traction_old(getMaterialPropertyOld<RealVectorValue>("traction")),
    _traction_local_old(getMaterialPropertyOld<RealVectorValue>("traction_local")),
    _traction_spatial_derivatives(declareProperty<RankTwoTensor>("traction_spatial_derivatives")),
    _traction_spatial_derivatives_local(
        declareProperty<RankTwoTensor>("traction_spatial_derivatives_local"))

{
}

void
CZMMaterialBase::computeQpProperties()
{

  RealTensorValue RotationGlobal2Local =
      RotationMatrix::rotVec1ToVec2(_normals[_qp], RealVectorValue(1, 0, 0));

  _displacement_jump[_qp](0) = _disp_x_neigh[_qp] - _disp_x[_qp];
  _displacement_jump[_qp](1) = _disp_y_neigh[_qp] - _disp_y[_qp];
  _displacement_jump[_qp](2) = _disp_z_neigh[_qp] - _disp_z[_qp];

  _displacement_jump_dot[_qp](0) = _disp_x_neigh_dot[_qp] - _disp_x_dot[_qp];
  _displacement_jump_dot[_qp](1) = _disp_y_neigh_dot[_qp] - _disp_y_dot[_qp];
  _displacement_jump_dot[_qp](2) = _disp_y_neigh_dot[_qp] - _disp_z_dot[_qp];

  _displacement_jump_local[_qp] = rotateVector(_displacement_jump[_qp], RotationGlobal2Local);

  _displacement_jump_dot_local[_qp] =
      rotateVector(_displacement_jump_dot[_qp], RotationGlobal2Local);

  _traction_local[_qp] = computeLocalTraction();
  _traction_spatial_derivatives_local[_qp] = computeLocalTractionDerivatives();

  _traction[_qp] = rotateVector(_traction_local[_qp], RotationGlobal2Local, /*inverse =*/true);
  _traction_spatial_derivatives[_qp] = rotateTensor2(
      _traction_spatial_derivatives_local[_qp], RotationGlobal2Local, /*inverse =*/true);
}

void
CZMMaterialBase::initQpStatefulProperties()
{
  RealTensorValue RotationGlobal2Local =
      RotationMatrix::rotVec1ToVec2(_normals[_qp], RealVectorValue(1, 0, 0));

  for (unsigned int i = 0; i < 3; i++)
  {
    _displacement_jump[_qp](i) = 0;
    _displacement_jump_local[_qp](i) = 0;
    _traction[_qp](i) = 0;
    _traction_local[_qp](i) = 0;
  }
}

RealVectorValue
CZMMaterialBase::rotateVector(const RealVectorValue v,
                              const RealTensorValue R,
                              const bool inverse /*= false*/)
{
  RealTensorValue R_loc = R;
  if (inverse)
    R_loc = R_loc.transpose();

  RealVectorValue vrot;

  for (unsigned int i = 0; i < 3; i++)
    for (unsigned int j = 0; j < 3; j++)
      vrot(i) += v(j) * R_loc(i, j);
  return vrot;
}

RankTwoTensor
CZMMaterialBase::rotateTensor2(const RankTwoTensor T,
                               const RealTensorValue R,
                               const bool inverse /*= false*/)
{
  RealTensorValue R_loc = R;
  if (inverse)
    R_loc = R_loc.transpose();

  RankTwoTensor trot = T;
  trot.rotate(R_loc);
  return trot;
}
