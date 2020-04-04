//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "XFEMRankTwoTensorMarkerUserObject.h"

#include "libmesh/quadrature.h"
#include "RankTwoTensor.h"
#include "RankTwoScalarTools.h"
#include "Assembly.h"
#include <limits>

registerMooseObject("XFEMApp", XFEMRankTwoTensorMarkerUserObject);

InputParameters
XFEMRankTwoTensorMarkerUserObject::validParams()
{
  InputParameters params = XFEMMaterialStateMarkerBase::validParams();
  params.addClassDescription(
      "Mark elements to be cut by XFEM based on a scalar extracted from a RankTwoTensor");
  params.addParam<MooseEnum>(
      "scalar_type",
      RankTwoScalarTools::scalarOptions(),
      "Scalar quantity to be computed from tensor and used as a failure criterion");
  params.addRequiredParam<std::string>("tensor", "The material tensor name.");
  params.addRequiredCoupledVar("threshold", "The threshold for crack growth.");
  params.addRequiredParam<bool>(
      "average", "Should the tensor quantity be averaged over the quadrature points?");
  params.addParam<Point>(
      "point1",
      Point(0, 0, 0),
      "Start point for axis used to calculate some cylindrical material tensor quantities");
  params.addParam<Point>(
      "point2",
      Point(0, 1, 0),
      "End point for axis used to calculate some cylindrical material tensor quantities");
  return params;
}

XFEMRankTwoTensorMarkerUserObject::XFEMRankTwoTensorMarkerUserObject(
    const InputParameters & parameters)
  : XFEMMaterialStateMarkerBase(parameters),
    _tensor(getMaterialProperty<RankTwoTensor>(getParam<std::string>("tensor"))),
    _scalar_type(getParam<MooseEnum>("scalar_type")),
    _point1(parameters.get<Point>("point1")),
    _point2(parameters.get<Point>("point2")),
    _threshold(coupledValue("threshold")),
    _average(getParam<bool>("average")),
    _JxW(_assembly.JxW()),
    _coord(_assembly.coordTransformation())
{
}

bool
XFEMRankTwoTensorMarkerUserObject::doesElementCrack(RealVectorValue & direction)
{
  bool does_it_crack = false;
  unsigned int numqp = _qrule->n_points();
  Point zero; // Used for checking whether direction is zero

  if (_average)
  {
    Real average_threshold = 0.0;
    RankTwoTensor average_tensor;
    Point average_point;
    for (unsigned int qp = 0; qp < numqp; ++qp)
    {
      if (_threshold[qp] <= 0.0)
        mooseError("Threshold must be strictly positive in XFEMRankTwoTensorMarkerUserObject");
      average_threshold += _JxW[qp] * _coord[qp] * _threshold[qp];
      average_tensor += _JxW[qp] * _coord[qp] * _tensor[qp];
      average_point += _JxW[qp] * _coord[qp] * _q_point[qp];
    }
    Point point_dir;
    Real tensor_quantity = RankTwoScalarTools::getQuantity(
        average_tensor, _scalar_type, _point1, _point2, average_point, point_dir);
    if (point_dir.absolute_fuzzy_equals(zero))
      mooseError("Direction has zero length in XFEMRankTwoTensorMarkerUserObject");
    direction = point_dir;
    if (tensor_quantity > average_threshold)
      does_it_crack = true;
  }
  else
  {
    unsigned int max_index = std::numeric_limits<unsigned int>::max();
    Real max_ratio = 0.0;
    std::vector<Point> directions(numqp);
    for (unsigned int qp = 0; qp < numqp; ++qp)
    {
      if (_threshold[qp] <= 0.0)
        mooseError("Threshold must be strictly positive in XFEMRankTwoTensorMarkerUserObject");
      const Real tensor_quantity = RankTwoScalarTools::getQuantity(
          _tensor[qp], _scalar_type, _point1, _point2, _q_point[qp], directions[qp]);
      if (directions[qp].absolute_fuzzy_equals(zero))
        mooseError("Direction has zero length in XFEMRankTwoTensorMarkerUserObject");
      Real ratio = tensor_quantity / _threshold[qp];
      if (ratio > max_ratio)
      {
        max_ratio = ratio;
        max_index = qp;
      }
    }
    if (max_ratio > 1.0)
    {
      if (max_index == std::numeric_limits<unsigned int>::max())
        mooseError("max_index out of bounds in XFEMRankTwoTensorMarkerUserObject");
      does_it_crack = true;
      direction = directions[max_index];
    }
  }

  return does_it_crack;
}
