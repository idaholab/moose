/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef XFEMRANKTWOTENSORMARKERUSEROBJECT_H
#define XFEMRANKTWOTENSORMARKERUSEROBJECT_H

#include "XFEMMaterialStateMarkerBase.h"

class XFEMRankTwoTensorMarkerUserObject;
class RankTwoTensor;

template <>
InputParameters validParams<XFEMRankTwoTensorMarkerUserObject>();

class XFEMRankTwoTensorMarkerUserObject : public XFEMMaterialStateMarkerBase
{
public:
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

#endif // XFEMRANKTWOTENSORMARKERUSEROBJECT_H
