//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PointInPolyhedronBaseUO.h"

InputParameters
PointInPolyhedronBaseUO::validParams()
{
  InputParameters params = GeneralUserObject::validParams();

  params.addParam<bool>("brute_force",
                        false,
                        "If true, use brute force to check if the point is inside the geometry "
                        "by loop over every elements.");

  params.addParam<Point>(
      "ray_direction", Point(0, 0, 0), "The direction of the ray for in-out testing.");

  params.addParam<Real>("eps",
                        libMesh::TOLERANCE,
                        "Tolerance value used for intersection or surface proximity checks. "
                        "This parameter determines whether a point is considered on the geometry "
                        "or on the in/out sides of the geometry.");

  params.addParam<int>(
      "leaf_max_size", 10, "Maximum number of elements in a leaf node of the KD-tree.");

  params.addParam<FileName>("obb_file_name", "", "Oriented Bounding Box (OBB) file name");
  params.addParam<FileName>("ray_file_name", "", "Ray file name");

  params.addClassDescription("Base class for user objects that determine if a point is inside a "
                             "polyhedron or polyhedra.");

  return params;
}

PointInPolyhedronBaseUO::PointInPolyhedronBaseUO(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    _ray_direction(parameters.get<Point>("ray_direction")),
    _brute_force(parameters.get<bool>("brute_force")),
    _eps(parameters.get<Real>("eps")),
    _leaf_max_size(parameters.get<int>("leaf_max_size")),
    _obb_file_name(parameters.get<FileName>("obb_file_name")),
    _ray_file_name(parameters.get<FileName>("ray_file_name"))
{
}
