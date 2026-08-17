//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SurfaceDelaunayGeneratorBase.h"
#include "libmesh/int_range.h"
#include "libmesh/parallel_implementation.h"
#include "libmesh/parallel_algebra.h"

#include <algorithm>

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

InputParameters
SurfaceDelaunayGeneratorBase::boundaryAndHolesParams()
{
  InputParameters params = emptyInputParameters();

  params.addRequiredParam<MeshGeneratorName>(
      "boundary",
      "The input MeshGenerator defining the output outer boundary and required Steiner points.");
  params.addParam<std::vector<BoundaryName>>(
      "input_boundary_names", "2D-input-mesh boundaries defining the output mesh outer boundary");
  params.addParam<std::vector<SubdomainName>>(
      "input_subdomain_names", "1D-input-mesh subdomains defining the output mesh outer boundary");
  params.addParam<unsigned int>("add_nodes_per_boundary_segment",
                                0,
                                "How many more nodes to add in each outer boundary segment.");
  params.addParam<bool>(
      "refine_boundary", true, "Whether to allow automatically refining the outer boundary.");

  params.addParam<SubdomainName>("output_subdomain_name",
                                 "Subdomain name to set on new triangles.");

  params.addParam<BoundaryName>(
      "output_boundary",
      "Boundary name to set on new outer boundary.  Default ID: 0 if no hole meshes are stitched; "
      "or maximum boundary ID of all the stitched hole meshes + 1.");
  params.addParam<std::vector<BoundaryName>>(
      "hole_boundaries",
      "Boundary names to set on holes.  Default IDs are numbered up from 1 if no hole meshes are "
      "stitched; or from maximum boundary ID of all the stitched hole meshes + 2.");

  params.addParam<bool>(
      "verify_holes",
      true,
      "Verify holes do not intersect boundary or each other.  Asymptotically costly.");

  params.addParam<std::vector<MeshGeneratorName>>(
      "holes", std::vector<MeshGeneratorName>(), "The MeshGenerators that define mesh holes.");
  params.addParam<std::vector<bool>>(
      "stitch_holes", std::vector<bool>(), "Whether to stitch to the mesh defining each hole.");
  params.addParam<std::vector<bool>>("refine_holes",
                                     std::vector<bool>(),
                                     "Whether to allow automatically refining each hole boundary.");
  params.addRangeCheckedParam<Real>(
      "desired_area",
      0,
      "desired_area>=0",
      "Desired (maximum) triangle area, or 0 to skip uniform refinement");
  params.addParam<std::string>(
      "desired_area_func",
      std::string(),
      "Desired area as a function of x,y; omit to skip non-uniform refinement");

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

void
SurfaceDelaunayGeneratorBase::checkBoundaryAndHolesParams(
    const std::vector<std::unique_ptr<MeshBase> *> & hole_ptrs) const
{
  const auto desired_area = getParam<Real>("desired_area");
  const auto & desired_area_func = getParam<std::string>("desired_area_func");

  if ((desired_area > 0.0 && !desired_area_func.empty()) ||
      (desired_area > 0.0 && _use_auto_area_func) ||
      (!desired_area_func.empty() && _use_auto_area_func))
    paramError("desired_area_func",
               "Only one of the three methods ('desired_area', 'desired_area_func', and "
               "'use_auto_area_func') to set element area limit should be used.");

  if (!_use_auto_area_func)
    if (isParamSetByUser("auto_area_func_default_size") ||
        isParamSetByUser("auto_area_func_default_size_dist") ||
        isParamSetByUser("auto_area_function_num_points") ||
        isParamSetByUser("auto_area_function_power"))
      paramError("use_auto_area_func",
                 "If this parameter is set to false, the following parameters should not be set: "
                 "'auto_area_func_default_size', 'auto_area_func_default_size_dist', "
                 "'auto_area_function_num_points', 'auto_area_function_power'.");

  const auto & stitch_holes = getParam<std::vector<bool>>("stitch_holes");
  const auto & refine_holes = getParam<std::vector<bool>>("refine_holes");

  if (!stitch_holes.empty() && stitch_holes.size() != hole_ptrs.size())
    paramError("stitch_holes", "Need one stitch_holes entry per hole, if specified.");

  for (const auto hole_i : index_range(stitch_holes))
    if (stitch_holes[hole_i] && (hole_i >= refine_holes.size() || refine_holes[hole_i]))
      paramError("refine_holes", "Disable auto refine of any hole boundary to be stitched.");

  if (isParamValid("hole_boundaries"))
    if (getParam<std::vector<BoundaryName>>("hole_boundaries").size() != hole_ptrs.size())
      paramError("hole_boundaries", "Need one hole_boundaries entry per hole, if specified.");
}

void
SurfaceDelaunayGeneratorBase::checkInteriorPoints(const std::vector<Point> & interior_points) const
{
  const bool has_duplicates = std::any_of(
      interior_points.begin(),
      interior_points.end(),
      [&interior_points](const Point & point)
      { return std::count(interior_points.begin(), interior_points.end(), point) > 1; });
  if (has_duplicates)
    paramError("interior_points", "Duplicate points were found in the provided interior points.");
}

void
SurfaceDelaunayGeneratorBase::fillDelaunayOptions(
    MeshTriangulationUtils::XYDelaunayOptions & opts) const
{
  if (isParamValid("input_boundary_names"))
    opts.input_boundary_names = getParam<std::vector<BoundaryName>>("input_boundary_names");
  if (isParamValid("input_subdomain_names"))
    opts.input_subdomain_names = getParam<std::vector<SubdomainName>>("input_subdomain_names");
  opts.add_nodes_per_boundary_segment = getParam<unsigned int>("add_nodes_per_boundary_segment");
  opts.refine_bdy = getParam<bool>("refine_boundary");
  opts.verify_holes = getParam<bool>("verify_holes");
  opts.desired_area = getParam<Real>("desired_area");
  opts.desired_area_func = getParam<std::string>("desired_area_func");
  opts.use_auto_area_func = _use_auto_area_func;
  opts.auto_area_func_default_size = _auto_area_func_default_size;
  opts.auto_area_func_default_size_dist = _auto_area_func_default_size_dist;
  opts.auto_area_function_num_points = _auto_area_function_num_points;
  opts.auto_area_function_power = _auto_area_function_power;
  opts.stitch_holes = getParam<std::vector<bool>>("stitch_holes");
  opts.refine_holes = getParam<std::vector<bool>>("refine_holes");

  if (isParamValid("output_subdomain_name"))
  {
    opts.has_output_subdomain_name = true;
    opts.output_subdomain_name = getParam<SubdomainName>("output_subdomain_name");
  }
  if (isParamValid("output_boundary"))
  {
    opts.has_output_boundary = true;
    opts.output_boundary = getParam<BoundaryName>("output_boundary");
  }
  if (isParamValid("hole_boundaries"))
    opts.hole_boundaries = getParam<std::vector<BoundaryName>>("hole_boundaries");
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
