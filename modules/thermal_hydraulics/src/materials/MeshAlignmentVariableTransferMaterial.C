//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MeshAlignmentVariableTransferMaterial.h"
#include "SubProblem.h"
#include "SystemBase.h"
#include "Assembly.h"
#include "ADUtils.h"
#include "MooseVariableBase.h"
#include "MeshAlignment.h"

#include "libmesh/node.h"
#include "libmesh/elem.h"

registerMooseObject("ThermalHydraulicsApp", MeshAlignmentVariableTransferMaterial);

InputParameters
MeshAlignmentVariableTransferMaterial::validParams()
{
  InputParameters params = Material::validParams();

  params.addClassDescription("Creates an AD material property for a variable transferred from the "
                             "boundary of a 2D mesh onto a 1D mesh.");

  // Have to use std::string to circumvent block restrictable testing
  params.addRequiredParam<std::string>("paired_variable", "The variable to get the value of.");
  params.addRequiredParam<MaterialPropertyName>(
      "property_name",
      "The name of the material property that will be "
      "declared that will represent the transferred variable.");
  params.addRequiredParam<MeshAlignment *>("_mesh_alignment", "Mesh alignment object");
  return params;
}

MeshAlignmentVariableTransferMaterial::MeshAlignmentVariableTransferMaterial(
    const InputParameters & parameters)
  : Material(parameters),
    _nl_sys(_subproblem.systemBaseNonlinear()),
    _serialized_solution(_nl_sys.currentSolution()),
    _paired_variable(
        _subproblem
            .getVariable(_tid, getParam<std::string>("paired_variable"), Moose::VAR_NONLINEAR)
            .number()),
    _mesh_alignment(*getParam<MeshAlignment *>("_mesh_alignment")),
    _prop(declareADProperty<Real>(getParam<MaterialPropertyName>("property_name"))),
    _phi(_assembly.fePhi<Real>(FEType(FIRST, LAGRANGE)))
{
}

void
MeshAlignmentVariableTransferMaterial::computeProperties()
{
  std::vector<ADReal> nodal_values;
  for (const auto i : _current_elem->node_index_range())
  {
    const Node & node = _current_elem->node_ref(i);

    // Assumes the variable you are coupling to is from the nonlinear system for now.
    const auto coupled_node_id = _mesh_alignment.getCoupledNodeID(node.id());
    const Node * const coupled_node = _mesh.nodePtr(coupled_node_id);
    const auto dof_number = coupled_node->dof_number(_nl_sys.number(), _paired_variable, 0);
    nodal_values.push_back((*_serialized_solution)(dof_number));
    Moose::derivInsert(nodal_values.back().derivatives(), dof_number, 1.0);
  }

  for (const auto qp : make_range(_qrule->n_points()))
  {
    _prop[qp] = 0;
    for (const auto i : _current_elem->node_index_range())
      _prop[qp] += nodal_values[i] * _phi[i][qp];
  }
}
