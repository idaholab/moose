//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SurfaceDelaunayGeneratorBase.h"
#include "libmesh/parallel_implementation.h"
#include "libmesh/parallel_algebra.h"

InputParameters
SurfaceDelaunayGeneratorBase::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addParam<bool>("use_auto_area_func",
                        false,
                        "Use the automatic area function for the triangle meshing region.");
  params.addParam<Real>(
      "auto_area_func_default_size",
      0,
      "Background size for automatic area function, or 0 to use non background size");
  params.addParam<Real>("auto_area_func_default_size_dist",
                        -1.0,
                        "Effective distance of background size for automatic area "
                        "function, or negative to use non background size");
  params.addParam<unsigned int>("auto_area_function_num_points",
                                10,
                                "Maximum number of nearest points used for the inverse distance "
                                "interpolation algorithm for automatic area function calculation.");
  params.addRangeCheckedParam<Real>(
      "auto_area_function_power",
      1.0,
      "auto_area_function_power>0",
      "Polynomial power of the inverse distance interpolation algorithm for automatic area "
      "function calculation.");

  params.addClassDescription("Base class for Delaunay mesh generators applied to a surface.");

  params.addParamNamesToGroup(
      "use_auto_area_func auto_area_func_default_size auto_area_func_default_size_dist "
      "auto_area_function_num_points auto_area_function_power",
      "Automatic triangle meshing area control");

  params.addRangeCheckedParam<Real>(
      "max_angle_deviation",
      60.0,
      "max_angle_deviation>0 & max_angle_deviation<90",
      "Maximum angle deviation from the global average normal vector in the input mesh.");
  params.addParam<bool>(
      "verbose", false, "Whether the generator should output additional information");
  return params;
}

SurfaceDelaunayGeneratorBase::SurfaceDelaunayGeneratorBase(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _use_auto_area_func(getParam<bool>("use_auto_area_func")),
    _auto_area_func_default_size(getParam<Real>("auto_area_func_default_size")),
    _auto_area_func_default_size_dist(getParam<Real>("auto_area_func_default_size_dist")),
    _auto_area_function_num_points(getParam<unsigned int>("auto_area_function_num_points")),
    _auto_area_function_power(getParam<Real>("auto_area_function_power")),
    _max_angle_deviation(getParam<Real>("max_angle_deviation")),
    _verbose(getParam<bool>("verbose"))
{
}

Point
SurfaceDelaunayGeneratorBase::elemNormal(const Elem & elem)
{
  mooseAssert(elem.n_vertices() == 3 || elem.n_vertices() == 4, "unsupported element type.");
  // Only the first three vertices are used to calculate the normal vector
  const Point & p0 = *elem.node_ptr(0);
  const Point & p1 = *elem.node_ptr(1);
  const Point & p2 = *elem.node_ptr(2);

  if (elem.n_vertices() == 4)
  {
    const Point & p3 = *elem.node_ptr(3);
    return ((p2 - p0).cross(p3 - p1)).unit();
  }

  return ((p2 - p1).cross(p0 - p1)).unit();
}

Point
SurfaceDelaunayGeneratorBase::meshNormal2D(const MeshBase & mesh)
{
  Point mesh_norm = Point(0.0, 0.0, 0.0);
  Real mesh_area = 0.0;

  // Check all the elements' normal vectors
  for (const auto & elem : mesh.active_local_element_ptr_range())
  {
    const Real elem_area = elem->volume();
    mesh_norm += elemNormal(*elem) * elem_area;
    mesh_area += elem_area;
  }
  mesh.comm().sum(mesh_norm);
  mesh.comm().sum(mesh_area);
  mesh_norm /= mesh_area;
  return mesh_norm.unit();
}

Real
SurfaceDelaunayGeneratorBase::meshNormalDeviation2D(const MeshBase & mesh,
                                                    const Point & global_norm)
{
  Real max_deviation(0.0);
  // Check all the elements' deviation from the global normal vector
  for (const auto & elem : mesh.active_local_element_ptr_range())
  {
    const Real elem_deviation = std::acos(global_norm * elemNormal(*elem)) / M_PI * 180.0;
    max_deviation = std::max(max_deviation, elem_deviation);
    if (_verbose && elem_deviation > _max_angle_deviation)
      _console << "Element " << elem->id() << " from subdomain ID " << elem->subdomain_id()
               << " has normal deviation: " << elem_deviation << std::endl;
  }
  mesh.comm().max(max_deviation);
  return max_deviation;
}
