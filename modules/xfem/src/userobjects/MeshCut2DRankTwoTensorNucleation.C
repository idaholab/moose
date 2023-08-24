//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MeshCut2DRankTwoTensorNucleation.h"

#include "libmesh/quadrature.h"
#include "RankTwoTensor.h"
#include "RankTwoScalarTools.h"
#include "Assembly.h"
#include <limits>

registerMooseObject("XFEMApp", MeshCut2DRankTwoTensorNucleation);

InputParameters
MeshCut2DRankTwoTensorNucleation::validParams()
{
  InputParameters params = MeshCut2DNucleationBase::validParams();
  params.addClassDescription(
      "Nucleate a crack in MeshCut2D UO based on a scalar extracted from a RankTwoTensor");
  params.addParam<MooseEnum>(
      "scalar_type",
      RankTwoScalarTools::scalarOptions(),
      "Scalar quantity to be computed from tensor and used as a failure criterion");
  params.addRequiredParam<std::string>("tensor", "The material tensor name.");
  params.addRequiredCoupledVar(
      "nucleation_threshold",
      "Threshold for the scalar quantity of the RankTwoTensor to nucleate new cracks");
  params.addRequiredParam<Real>("nucleation_length", "Nucleated crack length.");
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

MeshCut2DRankTwoTensorNucleation::MeshCut2DRankTwoTensorNucleation(
    const InputParameters & parameters)
  : MeshCut2DNucleationBase(parameters),
    _tensor(getMaterialProperty<RankTwoTensor>(getParam<std::string>("tensor"))),
    _nucleation_threshold(coupledValue("nucleation_threshold")),
    _scalar_type(getParam<MooseEnum>("scalar_type")),
    _point1(parameters.get<Point>("point1")),
    _point2(parameters.get<Point>("point2")),
    _nucleation_length(getParam<Real>("nucleation_length")),
    _JxW(_assembly.JxW()),
    _coord(_assembly.coordTransformation())
{
}

bool
MeshCut2DRankTwoTensorNucleation::doesElementCrack(
    std::pair<RealVectorValue, RealVectorValue> & cutterElemNodes)
{
  bool does_it_crack = false;
  unsigned int numqp = _qrule->n_points();
  Point zero; // Used for checking whether cutter Element is zero length

  Real average_threshold = 0.0;
  RankTwoTensor average_tensor;
  Point average_point;
  for (unsigned int qp = 0; qp < numqp; ++qp)
  {
    if (_nucleation_threshold[qp] <= 0.0)
      mooseError("Threshold must be strictly positive in MeshCut2DRankTwoTensorNucleation");
    average_threshold += _JxW[qp] * _coord[qp] * _nucleation_threshold[qp];
    average_tensor += _JxW[qp] * _coord[qp] * _tensor[qp];
    average_point += _JxW[qp] * _coord[qp] * _q_point[qp];
  }
  Point point_dir;
  Real tensor_quantity = RankTwoScalarTools::getQuantity(
      average_tensor, _scalar_type, _point1, _point2, average_point, point_dir);
  if (point_dir.absolute_fuzzy_equals(zero))
    mooseError("Cutter element has zero length in MeshCut2DRankTwoTensorNucleation");

  if (tensor_quantity > average_threshold)
  {
    const Point elem_center(_current_elem->vertex_average());
    does_it_crack = true;
    Point crack_dir = point_dir.cross(Point(0, 0, 1));
    RealVectorValue point_0 = elem_center - _nucleation_length / 2.0 * crack_dir.unit();
    RealVectorValue point_1 = elem_center + _nucleation_length / 2.0 * crack_dir.unit();
    cutterElemNodes = {point_0, point_1};
  }

  return does_it_crack;
}
