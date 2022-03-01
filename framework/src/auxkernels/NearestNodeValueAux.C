//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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
    mooseError("NearestNodeValueAux can only be used with one boundary at a time!");
}

Real
NearestNodeValueAux::computeValue()
{
  // Assumes the variable you are coupling to is from the nonlinear system for now.
  const Node * nearest = _nearest_node.nearestNode(_current_node->id());
  mooseAssert(nearest != NULL, "I do not have the nearest node for you");
  dof_id_type dof_number = nearest->dof_number(_nl_sys.number(), _paired_variable, 0);

  return (*_serialized_solution)(dof_number);
}
