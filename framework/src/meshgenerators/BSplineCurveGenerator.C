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
#include "LinearInterpolation.h"
#include "MooseUtils.h"
#include "MooseMeshUtils.h"
#include "BSpline.h"
#include "SplineUtils.h"

#include "libMeshReducedNamespace.h"

using namespace libMesh;

registerMooseObject("MooseApp", BSplineCurveGenerator);

InputParameters
BSplineCurveGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  MooseEnum edge_elem_type("EDGE2 EDGE3 EDGE4", "EDGE2");

  params.addParam<unsigned int>("degree", 3, "Degree of interpolating polynomial.");
  params.addRequiredParam<libMesh::Point>("start_point", "Starting (x,y,z) point for curve.");
  params.addRequiredParam<libMesh::Point>("end_point", "Ending (x,y,z) point for curve.");
  params.addRequiredParam<libMesh::RealVectorValue>("start_direction",
                                                    "Direction vector of curve at start point.");
  params.addRequiredParam<libMesh::RealVectorValue>("end_direction",
                                                    "Direction vector of curve at end point.");
  params.addRangeCheckedParam<libMesh::Real>(
      "sharpness", 0.6, "sharpness>0 & sharpness<=1", "Sharpness of curve bend.");
  params.addParam<unsigned int>(
      "num_cps",
      6,
      "Number of control points used to draw the curve. Miniumum of degree+1 points are required.");
  params.addParam<MooseEnum>(
      "edge_element_type", edge_elem_type, "Type of the EDGE elements to be generated.");
  params.addRequiredRangeCheckedParam<unsigned int>(
      "num_elements", "num_elements>=1", "Numer of elements to be drawn. Must be at least 1.");

  params.addClassDescription(
      "This BSplineMeshGenerator object is designed to generate a mesh of a curve that consists of "
      "EDGE2, EDGE3, or EDGE4 elements drawn using an open uniform B-Spline.");

  return params;
}

BSplineCurveGenerator::BSplineCurveGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _degree(getParam<unsigned int>("degree")),
    _start_point(getParam<libMesh::Point>("start_point")),
    _end_point(getParam<libMesh::Point>("end_point")),
    _start_dir(getParam<libMesh::RealVectorValue>("start_direction")),
    _end_dir(getParam<libMesh::RealVectorValue>("end_direction")),
    _sharpness(getParam<libMesh::Real>("sharpness")),
    _num_cps(getParam<unsigned int>("num_cps")),
    _order(getParam<MooseEnum>("edge_element_type") == "EDGE2"
               ? 1
               : (getParam<MooseEnum>("edge_element_type") == "EDGE3" ? 2 : 3)),
    _num_elements(getParam<unsigned int>("num_elements"))
{
  if (_num_cps < _degree + 1)
    paramError("num_cps", "Number of control points must be at least degree+1.");
}

std::unique_ptr<MeshBase>
BSplineCurveGenerator::generate()
{
  auto mesh = buildReplicatedMesh(2);

  // determine number of control points needed
  unsigned int half_cps;
  if (_num_cps % 2 == 0)
    half_cps = _num_cps / 2;
  else
  {
    half_cps = (_num_cps - 1) / 2;
    // add a mooseWarning
    mooseWarning("Need an even number of control points. `num_cps` has been increased by 1.");
  }

  // generate points using BSpline functions/class
  std::vector<Point> control_points = SplineUtils::bSplineControlPoints(
      _start_point, _end_point, _start_dir, _end_dir, half_cps, _sharpness);
  _console << Moose::stringify(control_points);

  // initialize BSpline class
  Moose::BSpline b_spline(
      _degree, _start_point, _end_point, _start_dir, _end_dir, half_cps, _sharpness);

  // discretize t and evaluate points, assemble into nodes inside loop
  unsigned int n_ts =
      _num_elements * _order + 1; // need to scale the number of elements by the order
  std::vector<Node *> nodes(n_ts); // store evaluated nodes
  libMesh::Real t_current;         // store current t (parameter) value
  std::vector<Point> eval_points; // store evaluated spline points
  for (const auto i : make_range(n_ts))
  {
    t_current = ((double)i / (double)(n_ts - 1)); // n_ts-1 because max(t_current) must be 1.0
    eval_points.push_back(
        b_spline.getPoint(t_current)); // calls BSpline public method to evaluate point
    nodes[i] = mesh->add_point(eval_points.back(), i); // get the most recent evaluation
  }

  // create elements from points
  for (const auto i : make_range(_num_elements))
  {
    std::unique_ptr<Elem> new_elem;
    new_elem = std::make_unique<Edge2>();
    if (_order > 1)
    {
      new_elem = std::make_unique<Edge3>();
      if (_order == 3)
      {
        new_elem = std::make_unique<Edge4>();
        new_elem->set_node(3, nodes[i * _order + 2]);
      }
      new_elem->set_node(2, nodes[i * _order + 1]);
    }
    new_elem->set_node(0, nodes[i * _order]);
    new_elem->set_node(1, nodes[((i + 1) * _order) % nodes.size()]);

    new_elem->subdomain_id() = 1; //
    mesh->add_elem(std::move(new_elem));
  }
  return dynamic_pointer_cast<MeshBase>(mesh);
}
