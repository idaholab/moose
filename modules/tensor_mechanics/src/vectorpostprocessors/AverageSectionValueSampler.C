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
#include <limits>

registerMooseObject("TensorMechanicsApp", AverageSectionValueSampler);

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
                        std::numeric_limits<double>::max(),
                        "Optional parameter to disambiguate cross sections of different structural "
                        "components when they share the same mesh block.");

  params.addRequiredParam<std::vector<VariableName>>(
      "variables", "Variables for the cross section output. These variables must be nodal.");
  params.addRequiredParam<std::vector<Real>>(
      "lengths", "Distance(s) to cross section from the global origin.");
  params.addParam<Real>("tolerance",
                        1.0e-6,
                        "Distance tolerance to identify nodes on the user-defined cross section.");
  return params;
}

AverageSectionValueSampler::AverageSectionValueSampler(const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters),
    _displaced_mesh(_app.actionWarehouse().displacedMesh()),
    _mesh(_app.actionWarehouse().mesh()),
    _variables(getParam<std::vector<VariableName>>("variables")),
    _direction(getParam<Point>("axis_direction")),
    _reference_point(getParam<Point>("reference_point")),
    _lengths(getParam<std::vector<Real>>("lengths")),
    _tolerance(getParam<Real>("tolerance")),
    _number_of_nodes(_lengths.size()),
    _cross_section_maximum_radius(getParam<Real>("cross_section_maximum_radius"))
{
  if (_mesh->dimension() != 3)
    mooseError("The AverageSectionValueSampler postprocessor can only be used with three "
               "dimensional meshes.");

  if (!MooseUtils::absoluteFuzzyEqual(_direction.norm_sq(), 1.0))
    paramError("axis_direction",
               "Axis diretion must have a norm of one and define the direction along which to "
               "locate cross sectional nodes.");

  _output_vector.resize(_variables.size());
  for (const auto j : make_range(_variables.size()))
    _output_vector[j] = &declareVector(_variables[j]);

  for (const auto j : make_range(_variables.size()))
  {
    const MooseVariable & variable = _sys.getFieldVariable<Real>(_tid, _variables[j]);
    if (!variable.isNodal())
      paramError(
          "variables",
          "The variables provided to this vector postprocessor must be defined at the nodes.");
  }
}

void
AverageSectionValueSampler::initialize()
{
  for (const auto j : make_range(_variables.size()))
  {
    _output_vector[j]->clear();
    _output_vector[j]->resize(_lengths.size());
  }
  _number_of_nodes.clear();
  _number_of_nodes.resize(_lengths.size());
}

void
AverageSectionValueSampler::finalize()
{
  for (const auto j : make_range(_variables.size()))
    _communicator.sum(*(_output_vector[j]));

  _communicator.sum(_number_of_nodes);

  for (const auto li : index_range(_lengths))
    if (_number_of_nodes[li] < 1)
      mooseError(
          "No nodes were found in AverageSectionValueSampler postprocessor. Revise your input.");

  for (const auto li : index_range(_lengths))
    for (const auto j : make_range(_variables.size()))
      (*_output_vector[j])[li] /= _number_of_nodes[li];
}

void
AverageSectionValueSampler::execute()
{
  _block_ids = _displaced_mesh->getSubdomainIDs(getParam<std::vector<SubdomainName>>("block"));

  auto * active_nodes = _mesh->getActiveSemiLocalNodeRange();

  std::vector<std::vector<Real>> output_vector_partial(_variables.size(),
                                                       std::vector<Real>(_lengths.size()));

  const NumericVector<Number> & _sol = *_sys.currentSolution();

  for (const auto & node : *active_nodes)
  {
    const std::set<SubdomainID> & node_blk_ids = _displaced_mesh->getNodeBlockIds(*node);

    for (const auto li : index_range(_lengths))
      for (const auto i : _block_ids)
        // If not contained in user blocks, continue
        if (node_blk_ids.find(i) != node_blk_ids.end())
        {
          // Check if node is close enough to user-prescribed plane
          if (distancePointPlane(*node, _direction, _reference_point, _lengths[li]) > _tolerance)
            continue;
          // Check node proc id is our processor id
          if ((*node).processor_id() != processor_id())
            continue;

          // Retrieve nodal variables
          for (const auto j : make_range(_variables.size()))
          {
            const MooseVariable & variable = _sys.getFieldVariable<Real>(_tid, _variables[j]);
            const auto var_num = variable.number();
            output_vector_partial[j][li] += _sol(node->dof_number(_sys.number(), var_num, 0));
          }

          _number_of_nodes[li]++;
        }
  }

  for (const auto j : make_range(_variables.size()))
    *(_output_vector[j]) = output_vector_partial[j];
}

Real
AverageSectionValueSampler::distancePointPlane(const Node & node,
                                               const Point & axis_direction,
                                               const Point & reference_point,
                                               const Real length) const
{
  // Compute node location w.r.t. structural component length
  const Point relative_distance{
      node(0) - reference_point(0), node(1) - reference_point(1), node(2) - reference_point(2)};

  const Real axial_distance = axis_direction(0) * relative_distance(0) +
                              axis_direction(1) * relative_distance(1) +
                              axis_direction(2) * relative_distance(2);
  const Real out_of_plane_distance =
      (relative_distance - relative_distance * axis_direction * axis_direction).norm();

  // If condition below is fulfilled, not in the user-defined structural component
  if (out_of_plane_distance > _cross_section_maximum_radius)
    return std::numeric_limits<double>::max();

  return std::abs(axial_distance - length) / std::sqrt(axis_direction * axis_direction);
}
