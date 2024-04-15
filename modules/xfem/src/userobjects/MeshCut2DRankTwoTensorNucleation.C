//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MeshCut2DRankTwoTensorNucleation.h"

#include "MooseError.h"
#include "MooseTypes.h"
#include "libmesh/quadrature.h"
#include "RankTwoTensor.h"
#include "RankTwoScalarTools.h"
#include "Assembly.h"

registerMooseObject("XFEMApp", MeshCut2DRankTwoTensorNucleation);

InputParameters
MeshCut2DRankTwoTensorNucleation::validParams()
{
  InputParameters params = MeshCut2DNucleationBase::validParams();
  params.addClassDescription(
      "Nucleate a crack in MeshCut2D UO based on a scalar extracted from a RankTwoTensor");
  params.addRangeCheckedParam<Real>(
      "edge_extension_factor",
      1e-5,
      "edge_extension_factor >= 0",
      "Crack length scaling factor to extend the nucleated crack beyond the cut element edges.");
  params.addRangeCheckedParam<Real>("nucleation_length",
                                    "nucleation_length >= 0",
                                    "Size of crack to nucleate.  Must be larger than the length of "
                                    "the element in which the crack is nucleated.");
  params.addParam<MooseEnum>(
      "scalar_type",
      RankTwoScalarTools::scalarOptions(),
      "Scalar quantity to be computed from tensor and used as a failure criterion");
  params.addRequiredParam<std::string>("tensor", "The material tensor name.");
  params.addRequiredCoupledVar(
      "nucleation_threshold",
      "Threshold for the scalar quantity of the RankTwoTensor to nucleate new cracks");
  params.addParam<Point>(
      "point1",
      Point(0, 0, 0),
      "Start point for axis used to calculate some cylindrical material tensor quantities");
  params.addParam<Point>(
      "point2",
      Point(0, 1, 0),
      "End point for axis used to calculate some cylindrical material tensor quantities");
  params.addParam<bool>(
      "always_cut_element",
      false,
      "Should element be cut if nucleation_length is smaller than element length.");
  return params;
}

MeshCut2DRankTwoTensorNucleation::MeshCut2DRankTwoTensorNucleation(
    const InputParameters & parameters)
  : MeshCut2DNucleationBase(parameters),
    _edge_extension_factor(getParam<Real>("edge_extension_factor")),
    _always_cut_element(getParam<bool>("always_cut_element")),
    _is_nucleation_length_provided(isParamValid("nucleation_length")),
    _nucleation_length(_is_nucleation_length_provided ? getParam<Real>("nucleation_length") : 0),
    _tensor(getMaterialProperty<RankTwoTensor>(getParam<std::string>("tensor"))),
    _nucleation_threshold(coupledValue("nucleation_threshold")),
    _scalar_type(getParam<MooseEnum>("scalar_type")),
    _point1(parameters.get<Point>("point1")),
    _point2(parameters.get<Point>("point2")),
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
    does_it_crack = true;

    const Point elem_center(_current_elem->vertex_average());
    Point crack_dir = point_dir.cross(Point(0, 0, 1));

    // get max element length to use for ray length
    std::unique_ptr<const Elem> edge;
    Real circumference = 0;
    for (const auto e : _current_elem->edge_index_range())
    {
      _current_elem->build_edge_ptr(edge, e);
      circumference += edge->length(0, 1);
    }

    // Temproraries for doing things below
    Real intersection_distance;

    Point point_0;
    Point point_1;
    bool is_point_0_on_external_boundary = false;
    bool is_point_1_on_external_boundary = false;
    for (const auto e : _current_elem->edge_index_range())
    {
      _current_elem->build_edge_ptr(edge, e);
      const auto & edge0 = edge->point(0);
      const auto & edge1 = edge->point(1);
      if (lineLineIntersect2D(
              elem_center, -crack_dir, circumference, edge0, edge1, point_0, intersection_distance))
        is_point_0_on_external_boundary = (_current_elem->neighbor_ptr(e) == nullptr);
      else if (lineLineIntersect2D(elem_center,
                                   crack_dir,
                                   circumference,
                                   edge0,
                                   edge1,
                                   point_1,
                                   intersection_distance))
        is_point_1_on_external_boundary = (_current_elem->neighbor_ptr(e) == nullptr);
    }

    // bisect_length is the length of a cut that goes across a single elment
    // extend_length is the amount that needs to be added to each side of the bisect_legnth cut to
    // make its length equal to the nucleation length
    // We also add a small amount to the crack length to make sure it fully cuts the element edge.
    // This factor is based on the element length.
    Real bisect_length = (point_0 - point_1).norm();
    Real extend_length = (_nucleation_length - bisect_length) / 2.0;
    if (_nucleation_length < bisect_length)
    {
      if (_is_nucleation_length_provided && !_always_cut_element)
        mooseError(
            "Trying to nucleate crack smaller than element length, increase nucleation_length.\n  "
            "location of crack being nucleated: ",
            point_0,
            "\n  nucleation_length: ",
            _nucleation_length,
            "\n  length to bisect element: ",
            bisect_length,
            "\n This error can be suppressed by setting always_cut_element=True which will "
            "nucleate a crack the size of the element.");
      else
        //_edge_extension_factor is used to make sure cut will cut both edges of the element.
        extend_length = (bisect_length * _edge_extension_factor) / 2.0;
    }

    // First create a cut assuming bulk nucleation
    point_0 = point_0 - extend_length * crack_dir.unit();
    point_1 = point_1 + extend_length * crack_dir.unit();

    // modify edges of cut should go so that they are on the edge of the domain and have a length
    // equal to nucleation_length
    if (is_point_0_on_external_boundary)
    {
      point_0 = point_0 - (bisect_length * _edge_extension_factor) / 2.0 * crack_dir.unit();
    }
    else if (is_point_1_on_external_boundary)
    {
      point_1 = point_1 + (bisect_length * _edge_extension_factor) / 2.0 * crack_dir.unit();
    }
    cutterElemNodes = {point_0, point_1};
  }

  return does_it_crack;
}

bool
MeshCut2DRankTwoTensorNucleation::lineLineIntersect2D(const Point & start,
                                                      const Point & direction,
                                                      const Real length,
                                                      const Point & v0,
                                                      const Point & v1,
                                                      Point & intersection_point,
                                                      Real & intersection_distance)
{
  /// The standard "looser" tolerance to use in ray tracing when having difficulty finding intersection
  const Real TRACE_TOLERANCE = 1e-5;

  const auto r = direction * length;
  const auto s = v1 - v0;
  const auto rxs = r(0) * s(1) - r(1) * s(0);

  // Lines are parallel or colinear
  if (std::abs(rxs) < TRACE_TOLERANCE)
    return false;

  const auto v0mu0 = v0 - start;
  const auto t = (v0mu0(0) * s(1) - v0mu0(1) * s(0)) / rxs;
  if (0 >= t + TRACE_TOLERANCE || t - TRACE_TOLERANCE > 1.0)
  {
    return false;
  }

  const auto u = (v0mu0(0) * r(1) - v0mu0(1) * r(0)) / rxs;
  if (0 < u + TRACE_TOLERANCE && u - TRACE_TOLERANCE <= 1.0)
  {
    intersection_point = start + r * t;
    intersection_distance = t * length;
    return true;
  }

  // Not parallel, but don't intersect
  return false;
}
