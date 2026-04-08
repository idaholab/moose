//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "BSplineCurveGenerator.h"
#include "CastUniquePointer.h"
#include "libMeshReducedNamespace.h"
#include "LinearInterpolation.h"
#include "MooseUtils.h"
#include "MooseMeshUtils.h"
#include "BSpline.h"
#include "SplineUtils.h"

#include "libmesh/edge_edge2.h"
#include "libmesh/edge_edge3.h"
#include "libmesh/edge_edge4.h"

using namespace libMesh;

registerMooseObject("MooseApp", BSplineCurveGenerator);

InputParameters
BSplineCurveGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  MooseEnum edge_elem_type("EDGE2 EDGE3 EDGE4", "EDGE2");

  // Convenience parameters
  params.addParam<SubdomainID>(
      "new_subdomain_id", 1, "Subdomain ID to assign to the curve elements");
  params.addParam<SubdomainName>("new_subdomain_name",
                                 "Subdomain name to assign to the curve elements");

  // Geometry parameters
  params.addParam<libMesh::Point>("start_point", "Starting (x,y,z) point for curve.");
  params.addParam<libMesh::Point>("end_point", "Ending (x,y,z) point for curve.");
  params.addParam<libMesh::RealVectorValue>("start_direction",
                                            "Direction vector of curve at start point.");
  params.addParam<libMesh::RealVectorValue>("end_direction",
                                            "Direction vector of curve at end point.");
  params.addParamNamesToGroup("start_point end_point start_direction end_direction",
                              "Curve extremities input");

  // Alternative to start / end point
  params.addParam<MeshGeneratorName>("start_mesh",
                                     "Meshgenerator providing the mesh to start spline from.");
  params.addParam<BoundaryName>("start_point_at_boundary_centroid",
                                "Boundary at whose centroid the spline should start.");
  params.addParam<MeshGeneratorName>("end_mesh",
                                     "Meshgenerator providing the mesh to end splne on.");
  params.addParam<BoundaryName>("end_point_at_boundary_centroid",
                                "Boundary at whose centroid the spline should end.");
  params.addParamNamesToGroup("start_mesh end_mesh start_point_at_boundary_centroid end_point_at_boundary_centroid",
                              "Curve extremities input");

  // Spline shape parameters
  params.addParam<unsigned int>("degree", 3, "Degree of interpolating polynomial.");
  params.addRangeCheckedParam<libMesh::Real>(
      "sharpness", 0.6, "sharpness>0 & sharpness<=1", "Sharpness of curve bend.");
  params.addParam<unsigned int>(
      "num_cps",
      6,
      "Number of control points used to draw the curve. Minimum of degree+1 points are required.");
  params.addParamNamesToGroup("degree sharpness num_cps", "Spline");

  // Discretization parameters
  params.addParam<MooseEnum>(
      "edge_element_type", edge_elem_type, "Type of the EDGE elements to be generated.");
  params.addRequiredRangeCheckedParam<unsigned int>(
      "num_elements", "num_elements>=1", "Numer of elements to be drawn. Must be at least 1.");
  params.addParamNamesToGroup("edge_element_type num_elements", "Discretization");

  params.addClassDescription(
      "This BSplineMeshGenerator object is designed to generate a mesh of a curve that consists of "
      "EDGE2, EDGE3, or EDGE4 elements drawn using an open uniform B-Spline.");

  return params;
}

BSplineCurveGenerator::BSplineCurveGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _new_subdomain_id(getParam<SubdomainID>("new_subdomain_id")),
    _degree(getParam<unsigned int>("degree")),
    _start_dir(getParam<libMesh::RealVectorValue>("start_direction")),
    _end_dir(getParam<libMesh::RealVectorValue>("end_direction")),
    _sharpness(getParam<libMesh::Real>("sharpness")),
    _num_cps(getParam<unsigned int>("num_cps")),
    _order((unsigned int)(getParam<MooseEnum>("edge_element_type")) + 1),
    _num_elements(getParam<unsigned int>("num_elements")),
    _start_mesh(getMesh("start_mesh", true)),
    _end_mesh(getMesh("end_mesh", true))
{
  if (_num_cps < _degree + 1)
    paramError("num_cps", "Number of control points must be at least degree+1.");

  // Check input parameters
  if (!isParamValid("start_point"))
  {
    if (!isParamValid("start_boundary"))
      paramError("start_boundary", "start_boundary must be specified if start_point is not");
    else if (!isParamValid("start_mesh"))
      paramError("start_mesh", "start_mesh must be specified if start_point is not.");
  }
  else
  {
    if (isParamValid("start_boundary") || isParamValid("start_mesh"))
      paramError(
          "start_point and start_boundary or start_mesh cannot be simultaneously specified!");
  }

  if (!isParamValid("end_point"))
  {
    if (!isParamValid("end_boundary"))
      paramError("end_boundary", "end_boundary must be specified if start_point is not");
    else if (!isParamValid("end_mesh"))
      paramError("end_mesh", "end_mesh must be specified if start_point is not.");
  }
  else
  {
    if (isParamValid("end_boundary") || isParamValid("end_mesh"))
      paramError("end_point and end_boundary or end_mesh cannot be simultaneously specified!");
  }
}

std::unique_ptr<MeshBase>
BSplineCurveGenerator::generate()
{
  const auto start_point = startPoint();
  const auto end_point = endPoint();

  auto mesh = buildReplicatedMesh(3);

  // determine number of control points needed
  unsigned int half_cps;
  if (_num_cps % 2 == 0)
    half_cps = _num_cps / 2;
  else
  {
    half_cps = (_num_cps - 1) / 2;
    mooseWarning("Need an even number of control points. `num_cps` has been decreased by 1.");
  }

  // generate points using BSpline functions/class
  std::vector<Point> control_points = SplineUtils::bSplineControlPoints(
      start_point, end_point, _start_dir, _end_dir, half_cps, _sharpness);

  // initialize BSpline class
  Moose::BSpline b_spline(
      _degree, start_point, end_point, _start_dir, _end_dir, half_cps, _sharpness);

  // discretize t and evaluate points, assemble into nodes inside loop
  const auto n_ts = _num_elements * _order + 1;
  std::vector<Node *> nodes(n_ts);
  std::vector<Point> eval_points;
  for (const auto i : make_range(n_ts))
  {
    // n_ts-1 because max(t_current) must be 1.0
    const auto t_current = ((Real)i / (Real)(n_ts - 1));
    eval_points.push_back(b_spline.getPoint(t_current));
    nodes[i] = mesh->add_point(eval_points.back(), i);
  }

  // create elements from points
  for (const auto i : make_range(_num_elements))
  {
    std::unique_ptr<Elem> new_elem;
    switch (_order)
    {
      case 1:
        new_elem = std::make_unique<Edge2>();
        break;
      case 2:
        new_elem = std::make_unique<Edge3>();
        new_elem->set_node(2, nodes[i * _order + 1]);
        break;
      default:
      {
        new_elem = std::make_unique<Edge4>();
        new_elem->set_node(2, nodes[i * _order + 1]);
        new_elem->set_node(3, nodes[i * _order + 2]);
      }
    }

    new_elem->set_node(0, nodes[i * _order]);
    mooseAssert((i + 1) * _order < nodes.size(), "Out of bounds in nodes array");
    new_elem->set_node(1, nodes[((i + 1) * _order)]);

    new_elem->subdomain_id() = _new_subdomain_id;
    mesh->add_elem(std::move(new_elem));
  }

  // Add subdomain name if needed
  if (isParamValid("new_subdomain_name"))
    mesh->subdomain_name(_new_subdomain_id) = getParam<SubdomainName>("new_subdomain_name");

  return dynamic_pointer_cast<MeshBase>(mesh);
}

libMesh::Point
BSplineCurveGenerator::startPoint()
{
  if (isParamValid("start_point"))
    return getParam<Point>("start_point");
  else
  {
    std::unique_ptr<MeshBase> start_mesh = std::move(_start_mesh);
    return MooseMeshUtils::boundaryCentroidCalculator(getParam<BoundaryName>("start_boundary"),
                                                      *start_mesh);
  }
}

libMesh::Point
BSplineCurveGenerator::endPoint()
{
  if (isParamValid("end_point"))
    return getParam<Point>("end_point");
  else
  {
    std::unique_ptr<MeshBase> end_mesh = std::move(_end_mesh);
    return MooseMeshUtils::boundaryCentroidCalculator(getParam<BoundaryName>("end_boundary"),
                                                      *end_mesh);
  }
}
