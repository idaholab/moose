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

  params.addRequiredParam<libMesh::Point>("start_point", "Starting (x,y,z) point for curve.");
  params.addRequiredParam<libMesh::Point>("end_point", "Ending (x,y,z) point for curve.");
  params.addRequiredParam<libMesh::RealVectorValue>("start_direction",
                                                    "Direction vector of curve at start point.");
  params.addRequiredParam<libMesh::RealVectorValue>("end_diretion",
                                                    "Direction vector of curve at end point.");
  params.addRequiredRangeCheckedParam<libMesh::Real>(
      "sharpness","sharpness>=0 && sharpness<=1", "Sharpness of curve bend.");
  params.addRequiredRangeCheckedParam<unsigned int>(
      "num_cps",
      "num_cps>=3",
      "Number of control points used to draw the curve. Miniumum of 3 points are required.");
  params.addRequiredParam<MooseEnum>(
      "edge_element_type", edge_elem_type, "Type of the EDGE elements to be generated.");
  params.addRequiredRangeCheckedParam<unsigned int>("num_elements","num_elements>=1", "Numer of elements to be drawn. Must be at least 1.");
  params.addParam<unsigned int>("degree",3,"Degree of interpolating polynomial.");

  params.addClassDescription("This BSplineMeshGenerator object is designed to generate a mesh of
                             a curve that consists of EDGE2, EDGE3, or EDGE4 elements drawn using 
                             an open uniform B-Spline.");

  return params;
}

BSplineCurveGenerator::BSplineCurveGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _start_point(getParam<libMesh::Point>("start_point")),
    _end_point(getParam<libMesh::Point>("end_point")),
    _start_dir(getParam<libMesh::RealVectorValue>("start_direction")),
    _end_dir(getParam<libMesh::RealVectorValue>("end_direction")),
    _sharpness(getParam<libMesh::Real>("sharpness")),
    _num_cps(getParam<unsigned int>("num_cps")),
    _order(getParam<MooseEnum>("edge_element_type") == "EDGE2"
               ? 1
               : (getParam<MooseEnum>("edge_element_type") == "EDGE3" ? 2 : 3))
    _num_elements(getParam<unsigned int>("num_elements"))
{
}

std::unique_ptr<MeshBase>
BSplineCurveGenerator::generate()
{
  auto mesh = libMesh::buildReplicatedMesh(2);

  // determine number of control points needed
  unsigned int half_cps;
  if (_num_cps%2==0) {
    half_cps = _num_cps/2;
    _num_cps +=1;
    // add a mooseWarning
    mooseWarning("Need an odd number of control points. num_cps has been increased by 1.");
  }
  else half_cps = (num_cps-1)/2;

  // generate points using BSpline functions/class
  std::vector<Point> control_points = SplineUtils::bSplineControlPoints(_start_point,_end_point,_start_dir,_end_dir,half_cps,_sharpness);

  // initialize BSpline class
  Moose::BSpline b_spline()

  // discretize t
  unsigned int n_ts = _num_elements+1;
  std::vector<double> ts;
  for (const auto i : make_range(n_ts))
    ts.push_back(i)/ (double)n_ts;


  // evaluate points at t

  // assemble into nodes

  std::vector<Node *> nodes(
      std::accumulate(_num_elements.begin(), _num_elements.end(), 0) * _order + 1);
  for (unsigned int i = 0; i < nodes.size(); i++)
  {
    _func_params[0] = linear_t->sample((Real)i);
    Point side_p = Point(evaluate(_func_Fx), evaluate(_func_Fy), evaluate(_func_Fz));
    nodes[i] = mesh->add_point(side_p, i);
  }

  // create elements from points
  const unsigned int num_elems = (nodes.size() - !_is_closed_loop) / _order;
  for (unsigned int i = 0; i < num_elems; i++)
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

    new_elem->subdomain_id() = 1;
    mesh->add_elem(std::move(new_elem));


    return dynamic_pointer_cast<MeshBase>(mesh);
  }
}

libMesh::Point
BSplineCurveGenerator::pointCalculator(const libMesh::Real t)
