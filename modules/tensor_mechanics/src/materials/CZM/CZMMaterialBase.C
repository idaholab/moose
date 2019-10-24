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
  params.addRequiredCoupledVar("displacements",
                               "The string of displacements suitable for the problem statement");
  return params;
}

CZMMaterialBase::CZMMaterialBase(const InputParameters & parameters)
  : InterfaceMaterial(parameters),
    _normals(_assembly.normals()),
    _ndisp(coupledComponents("displacements")),
    _disp(_ndisp),
    _disp_neighbor(_ndisp),
    _displacement_jump(declareProperty<RealVectorValue>("displacement_jump")),
    _displacement_jump_local(declareProperty<RealVectorValue>("displacement_jump_local")),
    _traction(declareProperty<RealVectorValue>("traction")),
    _traction_local(declareProperty<RealVectorValue>("traction_local")),
    _traction_spatial_derivatives(declareProperty<RankTwoTensor>("traction_spatial_derivatives")),
    _traction_spatial_derivatives_local(
        declareProperty<RankTwoTensor>("traction_spatial_derivatives_local"))
{
  if (_ndisp > 3 || _ndisp < 2)
    mooseError("the CZM material requires 2 or 3 disaplcment variables");

  // intializing disaplcement vectors
  for (unsigned int i = 0; i < _ndisp; ++i)
  {
    _disp[i] = &coupledValue("displacements", i);
    _disp_neighbor[i] = &coupledNeighborValue("displacements", i);
  }
  for (unsigned int i = _ndisp; i < 3; ++i)
  {
    _disp[i] = &_zero;
    _disp_neighbor[i] = &_zero;
  }
}

void
CZMMaterialBase::computeQpProperties()
{

  RealTensorValue RotationGlobal2Local =
      RotationMatrix::rotVec1ToVec2(_normals[_qp], RealVectorValue(1, 0, 0));

  for (unsigned int i = 0; i < 3; i++)
    _displacement_jump[_qp](i) = (*_disp_neighbor[i])[_qp] - (*_disp[i])[_qp];

  _displacement_jump_local[_qp] = rotateVector(_displacement_jump[_qp], RotationGlobal2Local);

  _traction_local[_qp] = computeLocalTraction();
  _traction_spatial_derivatives_local[_qp] = computeLocalTractionDerivatives();

  _traction[_qp] = rotateVector(_traction_local[_qp], RotationGlobal2Local, /*inverse =*/true);
  _traction_spatial_derivatives[_qp] = rotateTensor2(
      _traction_spatial_derivatives_local[_qp], RotationGlobal2Local, /*inverse =*/true);
}

RealVectorValue
CZMMaterialBase::rotateVector(const RealVectorValue v,
                              const RealTensorValue R,
                              const bool inverse /*= false*/)
{
  RealTensorValue R_loc = R;
  if (inverse)
    R_loc = R_loc.transpose();

  return R_loc * v;
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
