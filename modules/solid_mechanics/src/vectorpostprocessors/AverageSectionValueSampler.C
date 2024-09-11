//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AverageSectionValueSampler.h"
#include "MooseMesh.h"
#include "SystemBase.h"
#include "Conversion.h"
#include <limits>

registerMooseObject("SolidMechanicsApp", AverageSectionValueSampler);

InputParameters
AverageSectionValueSampler::validParams()
{
  InputParameters params = GeneralVectorPostprocessor::validParams();
  params.addClassDescription("Compute the section's variable average in three-dimensions "
                             "given a user-defined definition of the cross section.");
  params.addParam<std::vector<SubdomainName>>(
      "block",
      "The list of blocks in which to search for cross sectional nodes to compute the variable "
      "average.");
  params.addRequiredParam<Point>("axis_direction", "Direction of the structural component's axis");
  params.addRequiredParam<Point>("reference_point",
                                 "Structural component reference starting point from which the "
                                 "input parameter 'lengths' applies.");
  params.addParam<Real>("cross_section_maximum_radius",
                        std::numeric_limits<Real>::max(),
                        "Radial distance with respect to the body axis within which nodes are "
                        "considered to belong to this "
                        "structural component. Used to disambiguate multiple components that share "
                        "the same mesh block.");
  params.addRequiredParam<std::vector<VariableName>>(
      "variables", "Variables for the cross section output. These variables must be nodal.");
  params.addDeprecatedParam<std::vector<Real>>(
      "lengths",
      {},
      "Positions along axis from reference_point at which to compute average values.",
      "Use the 'positions' parameter instead");
  params.addParam<std::vector<Real>>(
      "positions",
      {},
      "Positions along axis from reference_point at which to compute average values.");
  params.addParam<RealVectorValue>(
      "symmetry_plane",
      RealVectorValue(0, 0, 0),
      "Vector normal to a symmetry plane, used if the section has a symmetry plane through it. "
      "Causes the variables to be treated as components of a vector and zeros out the compomnent "
      "in the direction normal to symmetry_plane. Three variables must be defined in 'variables' "
      "to use this option.");
  params.addParam<Real>("tolerance",
                        1.0e-6,
                        "Maximum axial distance of nodes from the specified axial lengths to "
                        "consider them in the cross-section average");
  params.addParam<bool>(
      "require_equal_node_counts",
      true,
      "Whether to require the number of nodes at each axial location to be equal");
  return params;
}

AverageSectionValueSampler::AverageSectionValueSampler(const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters),
    _mesh(_app.actionWarehouse().mesh()),
    _variables(getParam<std::vector<VariableName>>("variables")),
    _direction(getParam<Point>("axis_direction")),
    _reference_point(getParam<Point>("reference_point")),
    _positions(isParamSetByUser("lengths") ? getParam<std::vector<Real>>("lengths")
                                           : getParam<std::vector<Real>>("positions")),
    _have_symmetry_plane(isParamSetByUser("symmetry_plane")),
    _symmetry_plane(getParam<RealVectorValue>("symmetry_plane")),
    _automatically_locate_positions(_positions.size() == 0),
    _tolerance(getParam<Real>("tolerance")),
    _number_of_nodes(_positions.size()),
    _cross_section_maximum_radius(getParam<Real>("cross_section_maximum_radius")),
    _require_equal_node_counts(getParam<bool>("require_equal_node_counts")),
    _need_mesh_initializations(true)
{
  if (parameters.isParamSetByUser("lengths") && parameters.isParamSetByUser("positions"))
    paramError("lengths", "The 'lengths' and 'positions' parameters cannot both be set.");

  if (parameters.isParamSetByUser("lengths") && _positions.size() == 0)
    paramError("lengths", "If 'lengths' is specified, at least one value must be provided");

  if (parameters.isParamSetByUser("positions") && _positions.size() == 0)
    paramError("positions", "If 'positions' is specified, at least one value must be provided");

  if (_mesh->dimension() != 3)
    mooseError("The AverageSectionValueSampler postprocessor can only be used with three "
               "dimensional meshes.");

  if (!MooseUtils::absoluteFuzzyEqual(_direction.norm_sq(), 1.0))
    paramError("axis_direction", "Axis direction must be a unit vector.");

  _output_vector.resize(_variables.size() + 2);
  for (const auto j : make_range(_variables.size()))
    _output_vector[j] = &declareVector(_variables[j]);
  const auto pos_idx = _variables.size();
  _output_vector[pos_idx] = &declareVector("axial_position");
  _output_vector[pos_idx + 1] = &declareVector("node_count");

  for (const auto j : make_range(_variables.size()))
  {
    if (!_sys.hasVariable(_variables[j]))
      paramError("variables",
                 "One or more of the specified variables do not exist in this system.");
    const MooseVariable & variable = _sys.getFieldVariable<Real>(_tid, _variables[j]);
    if (!variable.isNodal())
      paramError("variables",
                 "One or more of the specified variables are not defined at the nodes.");
    _var_numbers.push_back(variable.number());
  }

  if (_have_symmetry_plane)
  {
    const auto symm_plane_norm = _symmetry_plane.norm();
    if (MooseUtils::absoluteFuzzyEqual(symm_plane_norm, 0.0))
      mooseError("Vector defined in 'symmetry_plane' cannot have a norm of zero");
    _symmetry_plane /= _symmetry_plane.norm();
    if (_variables.size() != 3)
      paramError("variables",
                 "If 'symmetry_plane' is prescribed, three variables must be provided (for the x, "
                 "y, z vector components, in that order)");
    mooseInfo("In AverageSectionValueSampler " + name() +
              ": \nTreating the variables as vector components (" + _variables[0] + ", " +
              _variables[1] + ", " + _variables[2] +
              ") and zeroing out the component normal to the prescribed 'symmetry_plane'\n");
  }
}

void
AverageSectionValueSampler::initialSetup()
{
}

void
AverageSectionValueSampler::meshChanged()
{
  _need_mesh_initializations = true;
}

void
AverageSectionValueSampler::initialize()
{
  if (_need_mesh_initializations)
  {
    // Option 1: locate positions first, then check counts
    if (_automatically_locate_positions)
      automaticallyLocatePositions();
    _need_mesh_initializations = false;
  }

  for (const auto j : make_range(_variables.size() + 2))
  {
    _output_vector[j]->clear();
    _output_vector[j]->resize(_positions.size());
  }
  _number_of_nodes.clear();
  _number_of_nodes.resize(_positions.size());
}

void
AverageSectionValueSampler::finalize()
{
  for (const auto j : make_range(_variables.size()))
    _communicator.sum(*(_output_vector[j]));

  _communicator.sum(_number_of_nodes);

  for (const auto li : index_range(_positions))
    if (_number_of_nodes[li] < 1)
      mooseError("No nodes were found in AverageSectionValueSampler postprocessor.");

  for (const auto li : index_range(_positions))
    for (const auto j : make_range(_variables.size()))
      (*_output_vector[j])[li] /= _number_of_nodes[li];

  const auto pos_idx = _variables.size();
  (*_output_vector[pos_idx]) = _positions;
  std::vector<Real> num_nodes_real(_number_of_nodes.begin(), _number_of_nodes.end());
  (*_output_vector[pos_idx + 1]) = num_nodes_real;

  if (_require_equal_node_counts)
  {
    for (const auto li : index_range(_number_of_nodes))
    {
      if (_number_of_nodes[li] != _number_of_nodes[0])
      {
        std::ostringstream pos_out;
        for (const auto li2 : index_range(_number_of_nodes))
          pos_out << std::setw(10) << _positions[li2] << std::setw(10) << _number_of_nodes[li2]
                  << "\n";
        mooseError(
            "Node counts do not match for all axial positions. If this behavior "
            "is desired to accommodate nonuniform meshes, set "
            "'require_equal_node_counts=false'\n     Axial      Node\n  Position     Count\n" +
            pos_out.str());
      }
    }
  }
}

void
AverageSectionValueSampler::execute()
{
  _block_ids = _mesh->getSubdomainIDs(getParam<std::vector<SubdomainName>>("block"));

  auto * active_nodes = _mesh->getActiveSemiLocalNodeRange();

  std::vector<std::vector<Real>> output_vector_partial(_variables.size(),
                                                       std::vector<Real>(_positions.size()));

  const NumericVector<Number> & _sol = *_sys.currentSolution();

  for (const auto & node : *active_nodes)
  {
    const std::set<SubdomainID> & node_blk_ids = _mesh->getNodeBlockIds(*node);

    for (const auto li : index_range(_positions))
    {
      for (const auto i : _block_ids)
      {
        if (node_blk_ids.find(i) != node_blk_ids.end())
        {
          // Check if node is within tolerance of user-prescribed plane
          if (std::abs(axialPosition(*node) - _positions[li]) > _tolerance)
            continue;
          if ((*node).processor_id() != processor_id())
            continue;

          // Retrieve nodal variables
          std::vector<Real> this_node_vars(_var_numbers.size());
          for (const auto j : make_range(_var_numbers.size()))
            this_node_vars[j] = _sol(node->dof_number(_sys.number(), _var_numbers[j], 0));

          if (_have_symmetry_plane)
          {
            RealVectorValue this_node_vec_var(
                this_node_vars[0], this_node_vars[1], this_node_vars[2]);
            this_node_vec_var -= _symmetry_plane * this_node_vec_var * _symmetry_plane;
            this_node_vars[0] = this_node_vec_var(0);
            this_node_vars[1] = this_node_vec_var(1);
            this_node_vars[2] = this_node_vec_var(2);
          }

          for (const auto j : make_range(_var_numbers.size()))
            output_vector_partial[j][li] += this_node_vars[j];

          _number_of_nodes[li]++;
          break;
        }
      }
    }
  }

  for (const auto j : make_range(_variables.size()))
    *_output_vector[j] = output_vector_partial[j];
}

Real
AverageSectionValueSampler::axialPosition(const Node & node) const
{
  // Compute node location w.r.t. structural component length
  const Point relative_distance{
      node(0) - _reference_point(0), node(1) - _reference_point(1), node(2) - _reference_point(2)};

  const Real axial_position = _direction * relative_distance;
  const Real in_plane_distance =
      (relative_distance - relative_distance * _direction * _direction).norm();

  // If the in-plane distance is greater than the specified cross-section radius, the point is not
  // in this component
  if (in_plane_distance > _cross_section_maximum_radius)
    return std::numeric_limits<Real>::max();

  return axial_position;
}

void
AverageSectionValueSampler::automaticallyLocatePositions()
{
  _positions.clear();

  // Data structure used to collect the locations with nodes on each processor,
  // and gather in parallel.
  std::vector<Real> pos_vec;

  // Data structure used to store parallel-gathered locations
  std::set<Real> pos_set;

  _block_ids = _mesh->getSubdomainIDs(getParam<std::vector<SubdomainName>>("block"));
  auto * nodes = _mesh->getLocalNodeRange();

  for (const auto node : *nodes)
  {
    const std::set<SubdomainID> & node_blk_ids = _mesh->getNodeBlockIds(*node);

    for (const auto i : _block_ids)
    {
      if (node_blk_ids.find(i) != node_blk_ids.end())
      {
        bool found_match = false;
        Real axial_position = axialPosition(*node);
        if (axial_position == std::numeric_limits<Real>::max()) // Node not in this component
          continue;
        for (auto & pos : pos_vec)
        {
          const Real dist_to_plane = std::abs(axial_position - pos);
          if (dist_to_plane <= _tolerance)
          {
            found_match = true;
            break;
          }
        }
        if (!found_match)
          pos_vec.emplace_back(axial_position);
      }
    }
  }

  _communicator.allgather(pos_vec);

  for (const auto & posv : pos_vec)
  {
    bool found_match = false;
    for (auto & poss : pos_set)
    {
      if (std::abs(posv - poss) < _tolerance)
      {
        found_match = true;
        break;
      }
    }
    if (!found_match)
      pos_set.insert(posv);
  }
  for (const auto & poss : pos_set)
    _positions.emplace_back(poss);
}
