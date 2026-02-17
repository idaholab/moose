//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ProjectSideSetOntoLevelSetGenerator.h"
#include "MooseMeshUtils.h"
#include "CastUniquePointer.h"

#include "libmesh/elem.h"
#include "libmesh/boundary_info.h"
#include "libmesh/mesh_base.h"

// C++ includes
#include <cmath>

registerMooseObject("MooseApp", ProjectSideSetOntoLevelSetGenerator);

InputParameters
ProjectSideSetOntoLevelSetGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  params += FunctionParserUtils<false>::validParams();

  params.addRequiredParam<Point>("direction", "Projection direction");
  params.addParam<SubdomainName>(
      "subdomain_name", "projected", "Name of the subdomain of the projected surface");
  params.addParam<Real>("max_search_distance",
                        1e6,
                        "Maximum distance from the sideset to search for the projected point in "
                        "the projection direction.");

  // Source domain parameters
  params.addRequiredParam<MeshGeneratorName>(
      "input", "The input mesh generator in which the sideset exists.");
  params.addRequiredParam<BoundaryName>("sideset", "The name of the sideset to project");

  // Surface definition
  params.addRequiredParam<std::string>(
      "level_set",
      "Level set to define the surface to project onto as a function of x, y, and z. The surface "
      "should be 'in front of' the sideset when following the projection direction.");
  params.addParam<std::vector<std::string>>(
      "constant_names", {}, "Vector of constants used in the parsed function");
  params.addParam<std::vector<std::string>>(
      "constant_expressions",
      {},
      "Vector of values for the constants in constant_names (can be an FParser expression)");
  params.addParamNamesToGroup("level_set constant_names constant_expressions", "Level set surface");

  params.addClassDescription(
      "Projects a sideset onto a surface defined by a level set and creates a surface mesh.");

  return params;
}

ProjectSideSetOntoLevelSetGenerator::ProjectSideSetOntoLevelSetGenerator(
    const InputParameters & parameters)
  : MeshGenerator(parameters),
    FunctionParserUtils<false>(parameters),
    _input_name(getParam<MeshGeneratorName>("input")),
    _input(getMeshByName(_input_name)),
    _level_set(getParam<std::string>("level_set")),
    _max_search_distance(getParam<Real>("max_search_distance"))
{
  // Create parsed function
  _func_level_set = std::make_shared<SymFunction>();
  parsedFunctionSetup(_func_level_set,
                      _level_set,
                      "x,y,z",
                      getParam<std::vector<std::string>>("constant_names"),
                      getParam<std::vector<std::string>>("constant_expressions"),
                      comm());
  _func_params.resize(3);

  // Get the projection direction and normalize it
  auto proj_d = getParam<Point>("direction");
  if (MooseUtils::absoluteFuzzyEqual(proj_d.norm_sq(), 0))
    paramError("direction", "Should not be 0");
  _proj_dir = proj_d.unit();
}

std::unique_ptr<MeshBase>
ProjectSideSetOntoLevelSetGenerator::generate()
{
  auto mesh_uptr = std::move(_input);
  auto mesh = mesh_uptr.get();
  auto new_mesh = buildMeshBaseObject(3);

  if (!mesh_uptr->is_serial())
    paramError("input", "Input mesh must not be distributed");

  // Use a new subdomain ID to avoid an overlap if merging the two later
  const auto projection_block_id = MooseMeshUtils::getNextFreeSubdomainID(*mesh);

  BoundaryInfo & boundary_info = mesh->get_boundary_info();
  const auto bdry_side_list = boundary_info.build_side_list();
  const auto side_id = boundary_info.get_id_by_name(getParam<BoundaryName>("sideset"));

  // Loop over the sides on a given set
  for (const auto & [elem_id, side_i, bc_id] : bdry_side_list)
  {
    if (bc_id != side_id)
      continue;

    // Find the intersection with the surface (projection) for each node of the side
    std::unique_ptr<Elem> side_elem = mesh->elem_ptr(elem_id)->build_side_ptr(side_i);
    const auto n_nodes = side_elem->n_nodes();
    const auto side_type = side_elem->type();
    std::vector<Node *> new_nodes(n_nodes);

    for (const auto i : make_range(n_nodes))
    {
      const auto start = side_elem->point(i);
      const auto end = start + _max_search_distance * _proj_dir;

      new_nodes[i] = new_mesh->add_point(pointPairLevelSetInterception(start, end));
    }

    // Build the same element as the side, on the surface, using the projected nodes
    if (side_type == libMesh::C0POLYGON)
      mooseError("Projection of polygonal sides is not supported at this time");
    std::unique_ptr<Elem> new_elem = Elem::build(side_type);
    for (const auto i : make_range(n_nodes))
      new_elem->set_node(i, new_nodes[i]);

    // User-selected block name: same for all element types for now
    new_elem->subdomain_id() = projection_block_id;

    new_mesh->add_elem(std::move(new_elem));
  }

  new_mesh->subdomain_name(projection_block_id) = getParam<SubdomainName>("subdomain_name");
  new_mesh->unset_is_prepared();
  return dynamic_pointer_cast<MeshBase>(new_mesh);
}

Point
ProjectSideSetOntoLevelSetGenerator::pointPairLevelSetInterception(const Point & point1,
                                                                   const Point & point2)
{
  Real dist1 = levelSetEvaluator(point1);
  Real dist2 = levelSetEvaluator(point2);

  if (MooseUtils::absoluteFuzzyEqual(dist1, 0.0) || MooseUtils::absoluteFuzzyEqual(dist2, 0.0))
    mooseError("At least one of the two points are on the plane.");
  if (MooseUtils::absoluteFuzzyGreaterThan(dist1 * dist2, 0.0))
    mooseError("The two points are on the same side of the plane.");

  Point p1(point1);
  Point p2(point2);
  Real dist = abs(dist1) + abs(dist2);
  Point mid_point;

  // Bisection method to find midpoint
  unsigned int num_its = 0;
  while (MooseUtils::absoluteFuzzyGreaterThan(dist, 0.0) && num_its < 1e3)
  {
    num_its++;
    mid_point = 0.5 * (p1 + p2);
    const Real dist_mid = levelSetEvaluator(mid_point);
    // We do not need Fuzzy here as it will be covered by the while loop
    if (dist_mid == 0.0)
      dist = 0.0;
    else if (dist_mid * dist1 < 0.0)
    {
      p2 = mid_point;
      dist2 = levelSetEvaluator(p2);
      dist = abs(dist1) + abs(dist2);
    }
    else
    {
      p1 = mid_point;
      dist1 = levelSetEvaluator(p1);
      dist = abs(dist1) + abs(dist2);
    }
  }

  if (num_its == 1e3)
    mooseError("Projection failed for point " + Moose::stringify(point1) +
               ". Is the level set 'in front of' the sideset when following the projection "
               "direction?");
  return mid_point;
}

Real
ProjectSideSetOntoLevelSetGenerator::levelSetEvaluator(const Point & point)
{
  _func_params[0] = point(0);
  _func_params[1] = point(1);
  _func_params[2] = point(2);
  return evaluate(_func_level_set);
}
