//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InterfaceMaterial.h"
class CZMMaterialBase;
template <>
InputParameters validParams<CZMMaterialBase>();
/**
 *
 */
class CZMMaterialBase : public InterfaceMaterial
{
public:
  CZMMaterialBase(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  const MooseArray<Point> & _normals;
  const unsigned int _ndisp;
  std::vector<const VariableValue *> _disp;
  std::vector<const VariableValue *> _disp_neighbor;

  virtual RealVectorValue computeLocalTraction() const = 0;
  virtual RankTwoTensor computeLocalTractionDerivatives() const = 0;

  /// the dispalcement jump in global coordiantes
  MaterialProperty<RealVectorValue> & _displacement_jump;

  /// the disaplcement jump in natural element coordiantes
  MaterialProperty<RealVectorValue> & _displacement_jump_local;

  /// the value of the Traction in global coordiantes
  MaterialProperty<RealVectorValue> & _traction;

  /// the value of the Traction in natural element coordiantes
  MaterialProperty<RealVectorValue> & _traction_local;

  /// the value of the traction derivatives in global coordiantes
  MaterialProperty<RankTwoTensor> & _traction_spatial_derivatives;

  /// the value of the traction derivatives in natural element coordiantes
  MaterialProperty<RankTwoTensor> & _traction_spatial_derivatives_local;

  /// Rotate a vector "T" via the rotation matrix "R".
  /// inverse rotation is achieved by setting "inverse" = true
  RealVectorValue rotateVector(const RealVectorValue /*V*/,
                               const RealTensorValue /*R*/,
                               const bool inverse = false);

  /// Rotate a rank2 tensor "T" via the rotation matrix "R".
  /// inverse rotation is achieved by setting "inverse" = true
  RankTwoTensor
  rotateTensor2(const RankTwoTensor /*T*/, const RealTensorValue /*R*/, const bool inverse = false);
};
