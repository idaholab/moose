//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MultiAppCopyTransfer.h"

// MOOSE includes
#include "DisplacedProblem.h"
#include "FEProblem.h"
#include "MooseMesh.h"
#include "MooseTypes.h"
#include "MooseVariable.h"
#include "MultiApp.h"
#include "NonlinearSystem.h"

#include "libmesh/system.h"
#include "libmesh/id_types.h"
#include "libmesh/string_to_enum.h"

template <>
InputParameters
validParams<MultiAppCopyTransfer>()
{
  InputParameters params = validParams<MultiAppTransfer>();
  params.addRequiredParam<VariableName>("variable",
                                        "The variable to store the transferred values in.");
  params.addRequiredParam<VariableName>("source_variable", "The variable to transfer from.");
  params.addClassDescription(
      "Copies variables (nonlinear and auxiliary) between multiapps that have identical meshes.");
  return params;
}

MultiAppCopyTransfer::MultiAppCopyTransfer(const InputParameters & parameters)
  : MultiAppTransfer(parameters),
    _to_var_name(getParam<VariableName>("variable")),
    _from_var_name(getParam<VariableName>("source_variable"))
{
}

void
MultiAppCopyTransfer::initialSetup()
{
  variableIntegrityCheck(_to_var_name);
}

void
MultiAppCopyTransfer::transferDofObject(libMesh::DofObject * to_object,
                                        libMesh::DofObject * from_object,
                                        MooseVariable & to_var,
                                        MooseVariable & from_var)
{
  if (to_object->n_dofs(to_var.sys().number(), to_var.number()) >
      0) // If this variable has dofs at this node
    for (unsigned int comp = 0; comp < to_object->n_comp(to_var.sys().number(), to_var.number());
         ++comp)
    {
      dof_id_type dof = to_object->dof_number(to_var.sys().number(), to_var.number(), comp);
      dof_id_type from_dof =
          from_object->dof_number(from_var.sys().number(), from_var.number(), comp);
      Real from_value = from_var.sys().solution()(from_dof);
      to_var.sys().solution().set(dof, from_value);
    }
}

void
MultiAppCopyTransfer::transfer(FEProblemBase & to_problem, FEProblemBase & from_problem)
{
  // Populate the to/from variables needed to perform the transfer
  MooseVariable & to_var = to_problem.getVariable(0, _to_var_name);
  MeshBase & to_mesh = to_problem.mesh().getMesh();

  MooseVariable & from_var = from_problem.getVariable(0, _from_var_name);
  MeshBase & from_mesh = from_problem.mesh().getMesh();

  // Check integrity
  if (to_var.feType() != from_var.feType())
    paramError("variable",
               "'variable' and 'source_variable' must be the same type (order and family): ",
               libMesh::Utility::enum_to_string<FEFamily>(to_var.feType().family),
               moose::internal::incompatVarMsg(to_var, from_var));

  if ((to_mesh.n_nodes() != from_mesh.n_nodes()) || (to_mesh.n_elem() != from_mesh.n_elem()))
    mooseError("The meshes must be identical to utilize MultiAppCopyTransfer.");

  // Transfer node dofs
  MeshBase::const_node_iterator node_it = to_mesh.local_nodes_begin();
  MeshBase::const_node_iterator node_end = to_mesh.local_nodes_end();
  for (; node_it != node_end; ++node_it)
    transferDofObject(*node_it, from_mesh.node_ptr((*node_it)->id()), to_var, from_var);

  // Transfer elem dofs
  MeshBase::const_element_iterator elem_it = to_mesh.local_elements_begin();
  MeshBase::const_element_iterator elem_end = to_mesh.local_elements_end();
  Elem * to_elem;
  Elem * from_elem;
  for (; elem_it != elem_end; ++elem_it)
  {
    to_elem = *elem_it;
    from_elem = from_mesh.elem_ptr(to_elem->id());
    mooseAssert(to_elem->type() == from_elem->type(), "The elements must be the same type.");
    transferDofObject(to_elem, from_elem, to_var, from_var);
  }

  to_var.sys().solution().close();
  to_var.sys().update();
}

void
MultiAppCopyTransfer::execute()
{
  _console << "Beginning MultiAppCopyTransfer " << name() << std::endl;

  if (_direction == TO_MULTIAPP)
  {
    FEProblemBase & from_problem = _multi_app->problemBase();
    for (unsigned int i = 0; i < _multi_app->numGlobalApps(); i++)
      if (_multi_app->hasLocalApp(i))
        transfer(_multi_app->appProblemBase(i), from_problem);
  }

  else if (_direction == FROM_MULTIAPP)
  {
    FEProblemBase & to_problem = _multi_app->problemBase();
    for (unsigned int i = 0; i < _multi_app->numGlobalApps(); i++)
      if (_multi_app->hasLocalApp(i))
        transfer(to_problem, _multi_app->appProblemBase(i));
  }

  _console << "Finished MultiAppCopyTransfer " << name() << std::endl;
}
