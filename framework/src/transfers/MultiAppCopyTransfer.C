/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

// MOOSE includes
#include "MultiAppCopyTransfer.h"
#include "MooseTypes.h"
#include "FEProblem.h"
#include "DisplacedProblem.h"
#include "MultiApp.h"
#include "MooseMesh.h"
#include "NonlinearSystem.h"

// libMesh includes
#include "libmesh/system.h"
#include "libmesh/mesh_tools.h"
#include "libmesh/id_types.h"

template<>
InputParameters validParams<MultiAppCopyTransfer>()
{
  InputParameters params = validParams<MultiAppTransfer>();
  params.addDeprecatedParam<AuxVariableName>("variable", "The auxiliary variable to store the transferred values in.", "This is being replaced by 'to_variable'.");
  params.addDeprecatedParam<VariableName>("source_variable", "The variable to transfer from.", "This is replaced by 'from_variable'.");
  params.addRequiredParam<VariableName>("to_variable", "The variable that the solution is being transfered into.");
  params.addRequiredParam<VariableName>("from_variable", "The variable to transfer from.");
  params.addClassDescription("Copies variables (nonlinear and auxiliary) between multiapps that have identical meshes.");
  return params;
}

MultiAppCopyTransfer::MultiAppCopyTransfer(const InputParameters & parameters) :
    MultiAppTransfer(parameters),
    _to_var_name(isParamValid("variable") ? getParam<VariableName>("variable") : getParam<VariableName>("to_variable")),
    _from_var_name(isParamValid("source_variable") ? getParam<VariableName>("source_variable") : getParam<VariableName>("from_variable"))
{
}

void
MultiAppCopyTransfer::initialSetup()
{
  variableIntegrityCheck(_to_var_name);
}

void
MultiAppCopyTransfer::transferDofObject()
{
  if (_to_object->n_dofs(_to_sys_num, _to_var_num) > 0) // If this variable has dofs at this node
    for (unsigned int comp = 0; comp < _to_object->n_comp(_to_sys_num, _to_var_num); ++comp)
    {
      dof_id_type dof = _to_object->dof_number(_to_sys_num, _to_var_num, comp);

      Moose::swapLibMeshComm(_swapped);

      dof_id_type from_dof = _from_object->dof_number(_from_sys_num, _from_var_num, comp);
      Real from_value = (*_from_sys->solution)(from_dof);

      _swapped = Moose::swapLibMeshComm(_multi_app->comm());

      _solution->set(dof, from_value);
    }
}

void
MultiAppCopyTransfer::transfer(FEProblem & to_problem, FEProblem & from_problem)
{
  // Populate the to/from variables needed to perform the transfer
  _swapped = Moose::swapLibMeshComm(_multi_app->comm());

  MooseVariable & to_var = to_problem.getVariable(0, _to_var_name);
  MooseVariable & from_var = from_problem.getVariable(0, _from_var_name);
  if (to_var.feType() != from_var.feType())
    mooseError("The variables must be the same type (order and family).");

  _from_sys = &from_var.sys().system();

  MeshBase * to_mesh = &to_problem.mesh().getMesh();
  MeshBase * from_mesh = &from_problem.mesh().getMesh();
  if ( (to_mesh->n_nodes() != from_mesh->n_nodes()) || (to_mesh->n_elem() != from_mesh->n_elem()))
    mooseError("The meshes must be identical to utilize MultiAppCopyTransfer.");

  _from_sys_num = _from_sys->number();
  _from_var_num = _from_sys->variable_number(from_var.name());

  System * to_sys = find_sys(to_problem.es(), _to_var_name);
  _to_sys_num = to_sys->number();
  _to_var_num = to_sys->variable_number(_to_var_name);

  _solution = to_problem.getNonlinearSystem().hasVariable(_to_var_name) ? &to_problem.getNonlinearSystem().solution() : &to_problem.getAuxiliarySystem().solution();

  // Transfer node dofs
  MeshBase::const_node_iterator node_it = to_mesh->local_nodes_begin();
  MeshBase::const_node_iterator node_end = to_mesh->local_nodes_end();
  for (; node_it != node_end; ++node_it)
  {
    _to_object = *node_it;
    _from_object = from_mesh->node_ptr(_to_object->id());
    transferDofObject();
  }

  // Transfer elem dofs
  MeshBase::const_element_iterator elem_it = to_mesh->local_elements_begin();
  MeshBase::const_element_iterator elem_end = to_mesh->local_elements_end();
  for (; elem_it != elem_end; ++elem_it)
  {
    _to_object = *elem_it;
    _from_object = from_mesh->elem_ptr(_to_object->id());
    mooseAssert( (*elem_it)->type() == (from_mesh->elem_ptr(_to_object->id()))->type(), "The elements must be the same type.");
    transferDofObject();
  }

  _solution->close();
  to_sys->update();

  // Swap back
  Moose::swapLibMeshComm(_swapped);
}

void
MultiAppCopyTransfer::execute()
{
  _console << "Beginning MultiAppCopyTransfer " << name() << std::endl;

  if (_direction == TO_MULTIAPP)
  {
    FEProblem & from_problem = _multi_app->problem();
    for (unsigned int i = 0; i < _multi_app->numGlobalApps(); i++)
    if (_multi_app->hasLocalApp(i))
      transfer(_multi_app->appProblem(i), from_problem);
  }

  else if (_direction == FROM_MULTIAPP)
  {
    FEProblem & to_problem = _multi_app->problem();
    for (unsigned int i = 0; i < _multi_app->numGlobalApps(); i++)
    if (_multi_app->hasLocalApp(i))
       transfer(to_problem, _multi_app->appProblem(i));
  }

  _console << "Finished MultiAppCopyTransfer " << name() << std::endl;
}
