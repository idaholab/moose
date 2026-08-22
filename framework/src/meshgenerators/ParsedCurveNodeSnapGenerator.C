//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ParsedCurveNodeSnapGenerator.h"
#include "ParsedCurveGenerator.h"
#include "MooseMeshUtils.h"

#include "libmesh/boundary_info.h"
#include "libmesh/elem.h"
#include "libmesh/mesh_base.h"
#include "libmesh/node.h"

// C++ includes
#include <algorithm>
#include <cmath>
#include <limits>
#include <set>

registerMooseObject("MooseApp", ParsedCurveNodeSnapGenerator);

InputParameters
ParsedCurveNodeSnapGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addRequiredParam<MeshGeneratorName>(
      "input", "The input mesh whose boundary nodes are snapped onto the curve.");
  params.addRequiredParam<BoundaryName>(
      "boundary", "The name of the boundary whose nodes are snapped onto the curve.");
  params.addRequiredParam<MeshGeneratorName>(
      "curve_generator", "The ParsedCurveGenerator that defines the curve to snap the nodes onto.");
  params.addRangeCheckedParam<unsigned int>(
      "samples_per_section",
      50,
      "samples_per_section>=2",
      "Number of uniformly spaced samples of each section of the curve that are used to bracket "
      "the closest point of the curve.");

  params.addClassDescription(
      "Snaps the nodes of a boundary onto the parametric curve of a ParsedCurveGenerator to "
      "recover the geometry that the straight element edges of the input mesh approximate.");

  return params;
}

ParsedCurveNodeSnapGenerator::ParsedCurveNodeSnapGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input")),
    // The mesh of the curve is not used by this generator. It is requested so that the mesh
    // generator system builds the ParsedCurveGenerator before this generator, which reads the
    // parameters of that generator below and evaluates its curve.
    _curve_mesh(getMesh("curve_generator")),
    _curve_generator(curveGenerator()),
    _curve_params(_curve_generator.parameters()),
    _boundary_name(getParam<BoundaryName>("boundary")),
    _samples_per_section(getParam<unsigned int>("samples_per_section")),
    _section_bounding_t_values(_curve_params.get<std::vector<Real>>("section_bounding_t_values")),
    _is_closed_loop(_curve_params.get<bool>("is_closed_loop"))
{
  if (_section_bounding_t_values.size() < 2)
    paramError("curve_generator",
               "The ParsedCurveGenerator '",
               getParam<MeshGeneratorName>("curve_generator"),
               "' must have at least two 'section_bounding_t_values' to define a curve.");

  // Sample each section of the curve uniformly, leaving out the end of the section as it is the
  // start of the next one
  for (const auto i : make_range(_section_bounding_t_values.size() - 1))
  {
    const Real t_start = _section_bounding_t_values[i];
    const Real t_end = _section_bounding_t_values[i + 1];
    for (const auto j : make_range(_samples_per_section))
      _t_samples.push_back(t_start +
                           (t_end - t_start) * static_cast<Real>(j) / _samples_per_section);
  }
  // The end of the last section is the start of the first one on a closed loop, so it is only
  // sampled for an open curve
  if (!_is_closed_loop)
    _t_samples.push_back(_section_bounding_t_values.back());
}

std::unique_ptr<MeshBase>
ParsedCurveNodeSnapGenerator::generate()
{
  // The mesh generator system requires that every requested mesh is released here, and the mesh of
  // the curve is only requested for the dependency it creates
  _curve_mesh.reset();

  std::unique_ptr<MeshBase> mesh = std::move(_input);

  if (!mesh->is_serial())
    paramError("input", "Input mesh must not be distributed");

  if (!MooseMeshUtils::hasBoundaryNameOrID(*mesh, _boundary_name))
    paramError("boundary", "The boundary '", _boundary_name, "' does not exist in the input mesh.");
  const auto boundary_id = MooseMeshUtils::getBoundaryID(_boundary_name, *mesh);

  // The nodes of the boundary can be defined by its sides as well as by its nodeset entries
  const BoundaryInfo & boundary_info = mesh->get_boundary_info();
  const auto side_list = boundary_info.build_side_list();
  const auto node_list = boundary_info.build_node_list();
  std::set<dof_id_type> boundary_node_ids;
  for (const auto & [elem_id, side, side_boundary_id] : side_list)
    if (side_boundary_id == boundary_id)
    {
      const Elem & elem = mesh->elem_ref(elem_id);
      for (const auto local_node_id : elem.nodes_on_side(side))
        boundary_node_ids.insert(elem.node_id(local_node_id));
    }
  for (const auto & [node_id, node_boundary_id] : node_list)
    if (node_boundary_id == boundary_id)
      boundary_node_ids.insert(node_id);

  // The sampled curve points do not depend on the node being snapped, so they are evaluated once
  // here instead of once per node
  _sample_points.reserve(_t_samples.size());
  for (const auto t_sample : _t_samples)
    _sample_points.push_back(curvePoint(t_sample));

  for (const auto node_id : boundary_node_ids)
  {
    Node & node = mesh->node_ref(node_id);
    const Point snapped_point = curvePoint(closestParameter(node));
    // The curve is defined in the XY plane, so the out-of-plane coordinate is left alone
    node(0) = snapped_point(0);
    node(1) = snapped_point(1);
  }

  mesh->unset_has_cached_elem_data();
  mesh->clear_point_locator();

  return mesh;
}

ParsedCurveGenerator &
ParsedCurveNodeSnapGenerator::curveGenerator() const
{
  const MeshGenerator & curve_generator =
      _app.getMeshGenerator(getParam<MeshGeneratorName>("curve_generator"));

  const auto parsed_curve_generator = dynamic_cast<const ParsedCurveGenerator *>(&curve_generator);
  if (!parsed_curve_generator)
    paramError("curve_generator",
               "The mesh generator '",
               curve_generator.name(),
               "' is of type '",
               curve_generator.type(),
               "', but a ParsedCurveGenerator is required to define the curve.");

  // The mesh generator system only hands out const generators, and evaluating the curve stages the
  // parameter in the parser of the generator that owns it, the same const_cast that
  // MeshGeneratorSystem::getMeshGeneratorInternal() makes for the same reason
  return const_cast<ParsedCurveGenerator &>(*parsed_curve_generator);
}

Point
ParsedCurveNodeSnapGenerator::curvePoint(const Real t_param)
{
  return _curve_generator.pointCalculator(_is_closed_loop ? wrappedParameter(t_param) : t_param);
}

Real
ParsedCurveNodeSnapGenerator::squaredDistance(const Real t_param, const Point & point)
{
  return squaredDistance(curvePoint(t_param), point);
}

Real
ParsedCurveNodeSnapGenerator::squaredDistance(const Point & curve_point, const Point & point) const
{
  const Real dx = curve_point(0) - point(0);
  const Real dy = curve_point(1) - point(1);
  return dx * dx + dy * dy;
}

Real
ParsedCurveNodeSnapGenerator::closestParameter(const Point & point)
{
  // Sampling the whole curve brackets the closest point globally, so that the refinement below
  // does not converge onto a closest point of another part of the curve
  std::size_t closest_sample = 0;
  Real closest_squared_distance = std::numeric_limits<Real>::max();
  for (const auto i : index_range(_t_samples))
  {
    const Real sample_squared_distance = squaredDistance(_sample_points[i], point);
    if (sample_squared_distance < closest_squared_distance)
    {
      closest_squared_distance = sample_squared_distance;
      closest_sample = i;
    }
  }

  // The closest point of the curve lies between the two samples that neighbor the closest sample.
  // On a closed loop, the sample before the first one and the sample after the last one are found
  // across the seam, one period below and above the sampled t values.
  const Real t_period = _section_bounding_t_values.back() - _section_bounding_t_values.front();
  Real t_before;
  if (closest_sample > 0)
    t_before = _t_samples[closest_sample - 1];
  else
    t_before = _is_closed_loop ? _t_samples.back() - t_period : _t_samples.front();
  Real t_after;
  if (closest_sample + 1 < _t_samples.size())
    t_after = _t_samples[closest_sample + 1];
  else
    t_after = _is_closed_loop ? _t_samples.front() + t_period : _t_samples.back();

  return goldenSectionSearch(std::min(t_before, t_after), std::max(t_before, t_after), point);
}

Real
ParsedCurveNodeSnapGenerator::goldenSectionSearch(const Real t_lower,
                                                  const Real t_upper,
                                                  const Point & point)
{
  // Inverse of the golden ratio, which is the factor the bracket shrinks by in each iteration
  const Real inv_golden_ratio = (std::sqrt(5.0) - 1.0) / 2.0;
  // A fixed number of iterations makes the search identical for every node. It shrinks the bracket
  // to about 1e-12 of its initial size, which is well below the size of an element.
  constexpr unsigned int num_iterations = 60;

  Real lower = t_lower;
  Real upper = t_upper;
  Real t_1 = upper - inv_golden_ratio * (upper - lower);
  Real t_2 = lower + inv_golden_ratio * (upper - lower);
  Real squared_distance_1 = squaredDistance(t_1, point);
  Real squared_distance_2 = squaredDistance(t_2, point);

  for ([[maybe_unused]] const auto i : make_range(num_iterations))
  {
    if (squared_distance_1 < squared_distance_2)
    {
      upper = t_2;
      t_2 = t_1;
      squared_distance_2 = squared_distance_1;
      t_1 = upper - inv_golden_ratio * (upper - lower);
      squared_distance_1 = squaredDistance(t_1, point);
    }
    else
    {
      lower = t_1;
      t_1 = t_2;
      squared_distance_1 = squared_distance_2;
      t_2 = lower + inv_golden_ratio * (upper - lower);
      squared_distance_2 = squaredDistance(t_2, point);
    }
  }

  return (lower + upper) / 2.0;
}

Real
ParsedCurveNodeSnapGenerator::wrappedParameter(const Real t_param) const
{
  const Real t_min =
      std::min(_section_bounding_t_values.front(), _section_bounding_t_values.back());
  const Real t_max =
      std::max(_section_bounding_t_values.front(), _section_bounding_t_values.back());
  const Real t_range = t_max - t_min;

  return t_param - t_range * std::floor((t_param - t_min) / t_range);
}
