//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NearestNodeValueAux.h"

#include "SystemBase.h"
#include "NearestNodeLocator.h"

registerMooseObject("MooseApp", NearestNodeValueAux);

InputParameters
NearestNodeValueAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Retrieves a field value from the closest node on the paired boundary "
                             "and stores it on this boundary or block.");
  params.set<bool>("_dual_restrictable") = true;
  params.addRequiredParam<BoundaryName>("paired_boundary", "The boundary to get the value from.");
  params.addRequiredCoupledVar("paired_variable", "The variable to get the value of.");
  params.set<bool>("use_displaced_mesh") = true;
  return params;
}

NearestNodeValueAux::NearestNodeValueAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _nearest_node(
        getNearestNodeLocator(parameters.get<BoundaryName>("paired_boundary"), boundaryNames()[0])),
    _serialized_solution(_nl_sys.currentSolution()),
    _paired_variable(coupled("paired_variable"))
{
  if (boundaryNames().size() > 1)
    paramError("boundary", "NearestNodeValueAux can only be used with one boundary at a time!");
  if (boundaryNames().empty())
    paramError("boundary", "Should be specified on a boundary or group of boundaries");

  // Check that the paired variable is from the solution system
  if (_subproblem.hasAuxiliaryVariable("paired_variable"))
    paramError("paired_variable", "Paired variable should not be auxiliary");
  // Check that the paired variable has degrees of freedom on nodes (and thus can be sampled
  // directly at the dof index of the node)
  if (!getVar("paired_variable", 0)->hasDoFsOnNodes())
    paramError("paired_variable", "Paired variable does not have degrees of freedom on nodes");

  // TODO: avoid the runtime checks by making sure the paired variable is defined on the boundaries
}

Real
NearestNodeValueAux::computeValue()
{
  if (isNodal())
  {
    // Assumes the variable you are coupling to is from the nonlinear system for now.
    const Node * nearest = _nearest_node.nearestNode(_current_node->id());
    if (nearest == nullptr)
      mooseError("Could not locate the nearest node from node: ", _current_node->get_info());
    dof_id_type dof_number = nearest->dof_number(_nl_sys.number(), _paired_variable, 0);
    if (dof_number == libMesh::DofObject::invalid_id)
      mooseError("Paired variable does not seem to be defined on nearest node: ",
                 nearest->get_info());
    return (*_serialized_solution)(dof_number);
  }
  else
  {
    // Get a value for all the nodes on the boundary, then average them for the centroid value
    Real average = 0;
    Real sum_inv_dist = 0;
    for (const auto & node : _current_elem->node_ref_range())
    {
      const Node * nearest = _nearest_node.nearestNode(node.id());
      // Some of the element's nodes wont be on the boundary
      if (!nearest)
        continue;
      const auto dof_number = nearest->dof_number(_nl_sys.number(), _paired_variable, 0);
      const auto distance = _nearest_node.distance(node.id());

      if (dof_number == libMesh::DofObject::invalid_id)
        mooseError("Paired variable does not seem to be defined on nearest node: ",
                   nearest->get_info());

      // inverse distance weighting should be a fine default
      if (distance > 0)
      {
        average += (*_serialized_solution)(dof_number) / distance;
        sum_inv_dist += 1. / distance;
      }
      // if node and nearest nodes coincide, weight with 1
      else
      {
        average += (*_serialized_solution)(dof_number);
        sum_inv_dist += 1.;
      }
    }

    if (sum_inv_dist > 0)
      return average / sum_inv_dist;
    else
      return 0.;
  }
}
