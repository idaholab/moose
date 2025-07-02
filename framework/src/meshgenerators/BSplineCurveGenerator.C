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
                                                    "Direction of curve at start point.");
  params.addRequiredParam<libMesh::RealVectorValue>("end_diretion",
                                                    "Direction of curve at end point.");
  params.addRequiredRangeCheckedParam<libMesh::Real>(
      "sharpness", "sharpness>=0 && sharpness<=1", "Sharpness of curve bend.");
  params.addRequiredRangeCheckedParam<unsigned int>(
      "num_pts",
      "num_pts>=3",
      "Number of points to be drawn along the curve. Miniumum of 3 points are required.");
  params.addParam<MooseEnum>(
      "edge_element_type", edge_elem_type, "Type of the EDGE elements to be generated.");

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
    _num_pts(getParam<unsigned int>("num_pts")),
    _order(getParam<MooseEnum>("edge_element_type") == "EDGE2"
               ? 1
               : (getParam<MooseEnum>("edge_element_type") == "EDGE3" ? 2 : 3))
{
}
