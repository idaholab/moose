//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatStructure2DCouplerBCBase.h"
#include "MeshAlignment.h"

InputParameters
HeatStructure2DCouplerBCBase::validParams()
{
  InputParameters params = ADIntegratedBC::validParams();

  params.addRequiredParam<std::string>("coupled_variable",
                                       "The variable on the coupled heat structure boundary");
  params.addRequiredParam<MeshAlignment *>("_mesh_alignment", "Mesh alignment object");

  return params;
}

HeatStructure2DCouplerBCBase::HeatStructure2DCouplerBCBase(const InputParameters & parameters)
  : ADIntegratedBC(parameters),

    _coupled_variable_number(
        _subproblem
            .getVariable(_tid, getParam<std::string>("coupled_variable"), Moose::VAR_NONLINEAR)
            .number()),
    _mesh_alignment(*getParam<MeshAlignment *>("_mesh_alignment")),

    _nl_sys(_subproblem.systemBaseNonlinear()),
    _serialized_solution(_nl_sys.currentSolution())
{
}

ADReal
HeatStructure2DCouplerBCBase::computeCoupledTemperature() const
{
  ADReal T_coupled = 0;
  for (const auto j : _current_elem->node_index_range())
  {
    const auto node_id = (_current_elem->node_ref(j)).id();
    // This check is because we're looping over all of the nodes of the current
    // element, not just those on the boundary; we must exclude the interior nodes.
    if (_mesh_alignment.hasCoupledNodeID(node_id))
    {
      const auto neighbor_node_id = _mesh_alignment.getCoupledNodeID(node_id);
      const Node & neighbor_node = _mesh.nodeRef(neighbor_node_id);
      const auto dof_number =
          neighbor_node.dof_number(_nl_sys.number(), _coupled_variable_number, 0);
      ADReal T_node = (*_serialized_solution)(dof_number);
      Moose::derivInsert(T_node.derivatives(), dof_number, 1.0);
      T_coupled += T_node * _test[j][_qp];
    }
  }

  return T_coupled;
}
