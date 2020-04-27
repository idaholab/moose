//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "XFEMMaterialStateMarkerBase.h"
#include "RankTwoTensorForward.h"

class XFEMRankTwoTensorMarkerUserObject : public XFEMMaterialStateMarkerBase
{
public:
  static InputParameters validParams();

  XFEMRankTwoTensorMarkerUserObject(const InputParameters & parameters);
  virtual ~XFEMRankTwoTensorMarkerUserObject() {}

protected:
  /// The tensor from which the scalar quantity used as a marking criterion is extracted
  const MaterialProperty<RankTwoTensor> & _tensor;

  /// The type of scalar to be extracted from the tensor
  MooseEnum _scalar_type;

  /// Points used to define an axis of rotation for some scalar quantities
  const Point _point1;
  const Point _point2;

  /// Threshold value of the scalar
  const VariableValue & _threshold;

  /// Whether to average the value for all quadrature points in an element
  bool _average;

  /// Transformed Jacobian weights
  const MooseArray<Real> & _JxW;
  const MooseArray<Real> & _coord;

  virtual bool doesElementCrack(RealVectorValue & direction) override;
};
