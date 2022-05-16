//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatStructure2DCouplerBC.h"
#include "MeshAlignment2D2D.h"
#include "Function.h"

registerMooseObject("ThermalHydraulicsApp", HeatStructure2DCouplerBC);

InputParameters
HeatStructure2DCouplerBC::validParams()
{
  InputParameters params = ADIntegratedBC::validParams();

  params.addRequiredParam<FunctionName>("heat_transfer_coefficient",
                                        "Heat transfer coefficient function");
  params.addRequiredParam<std::string>("coupled_variable",
                                       "The variable on the coupled heat structure boundary");
  params.addRequiredParam<MeshAlignment2D2D *>("_mesh_alignment", "Mesh alignment object");

  params.addClassDescription("Applies BC for HeatStructure2DCoupler for plate heat structure");

  return params;
}

HeatStructure2DCouplerBC::HeatStructure2DCouplerBC(const InputParameters & parameters)
  : ADIntegratedBC(parameters),

    _htc(getFunction("heat_transfer_coefficient")),
    _coupled_variable_number(
        _subproblem
            .getVariable(_tid, getParam<std::string>("coupled_variable"), Moose::VAR_NONLINEAR)
            .number()),
    _mesh_alignment(*getParam<MeshAlignment2D2D *>("_mesh_alignment")),

    _nl_sys(_subproblem.systemBaseNonlinear()),
    _serialized_solution(_nl_sys.currentSolution())
{
}

ADReal
HeatStructure2DCouplerBC::computeQpResidual()
{
  // Compute temperature of neighboring side
  ADReal T_coupled_hs = 0;
  for (const auto j : _current_elem->node_index_range())
  {
    const auto node_id = (_current_elem->node_ref(j)).id();
    if (_mesh_alignment.hasNeighborNode(node_id))
    {
      const auto neighbor_node_id = _mesh_alignment.getNeighborNodeID(node_id);
      const Node & neighbor_node = _mesh.nodeRef(neighbor_node_id);
      const auto dof_number =
          neighbor_node.dof_number(_nl_sys.number(), _coupled_variable_number, 0);
      ADReal T_node = (*_serialized_solution)(dof_number);
      Moose::derivInsert(T_node.derivatives(), dof_number, 1.0);
      T_coupled_hs += T_node * _test[j][_qp];
    }
  }

  return _htc.value(_t, _q_point[_qp]) * (_u[_qp] - T_coupled_hs) * _test[_i][_qp];
}
