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

#include "MooseApp.h"
#include "SystemBase.h"
#include "Factory.h"
#include "SubProblem.h"
#include "MooseVariable.h"
#include "Conversion.h"
#include "Parser.h"
#include "AllLocalDofIndicesThread.h"
#include "MooseTypes.h"
#include "InitialCondition.h"
#include "ScalarInitialCondition.h"
#include "Assembly.h"
#include "MooseMesh.h"

/// Free function used for a libMesh callback
void extraSendList(std::vector<dof_id_type> & send_list, void * context)
{
  SystemBase * sys = static_cast<SystemBase *>(context);
  sys->augmentSendList(send_list);
}

/// Free function used for a libMesh callback
void extraSparsity(SparsityPattern::Graph & sparsity,
                   std::vector<dof_id_type> & n_nz,
                   std::vector<dof_id_type> & n_oz,
                   void * context)
{
  SystemBase * sys = static_cast<SystemBase *>(context);
  sys->augmentSparsity(sparsity, n_nz, n_oz);
}

template<>
void
dataStore(std::ostream & stream, SystemBase & system_base, void * context)
{
  System & libmesh_system = system_base.system();

  NumericVector<Real> & solution = *(libmesh_system.solution.get());

  dataStore(stream, solution, context);

  for (System::vectors_iterator it = libmesh_system.vectors_begin();
       it != libmesh_system.vectors_end();
       it++)
    dataStore(stream, *(it->second), context);
}

template<>
void
dataLoad(std::istream & stream, SystemBase & system_base, void * context)
{
  System & libmesh_system = system_base.system();

  NumericVector<Real> & solution = *(libmesh_system.solution.get());

  dataLoad(stream, solution, context);

  for (System::vectors_iterator it = libmesh_system.vectors_begin();
       it != libmesh_system.vectors_end();
       it++)
    dataLoad(stream, *(it->second), context);

  system_base.update();
}

SystemBase::SystemBase(SubProblem & subproblem, const std::string & name) :
    libMesh::ParallelObject(subproblem),
    _subproblem(subproblem),
    _app(subproblem.getMooseApp()),
    _factory(_app.getFactory()),
    _mesh(subproblem.mesh()),
    _name(name),
    _vars(libMesh::n_threads()),
    _var_map()
{
}

MooseVariable &
SystemBase::getVariable(THREAD_ID tid, const std::string & var_name)
{
  MooseVariable * var = dynamic_cast<MooseVariable *>(_vars[tid].getVariable(var_name));
  if (var == NULL)
    mooseError("Variable '" + var_name + "' does not exist in this system");
  return *var;
}

MooseVariable &
SystemBase::getVariable(THREAD_ID tid, unsigned int var_number)
{
  MooseVariable * var = dynamic_cast<MooseVariable *>(_vars[tid].getVariable(var_number));
  if (var == NULL)
    mooseError("variable #" + Moose::stringify(var_number) + " does not exist in this system");
  return *var;
}

MooseVariableScalar &
SystemBase::getScalarVariable(THREAD_ID tid, const std::string & var_name)
{
  MooseVariableScalar * var = dynamic_cast<MooseVariableScalar *>(_vars[tid].getVariable(var_name));
  if (var == NULL)
    mooseError("Scalar variable '" + var_name + "' does not exist in this system");
  return *var;
}

MooseVariableScalar &
SystemBase::getScalarVariable(THREAD_ID tid, unsigned int var_number)
{
  MooseVariableScalar * var = dynamic_cast<MooseVariableScalar *>(_vars[tid].getVariable(var_number));
  if (var == NULL)
    mooseError("variable #" + Moose::stringify(var_number) + " does not exist in this system");
  return *var;
}

const std::set<SubdomainID> *
SystemBase::getVariableBlocks(unsigned int var_number)
{
  mooseAssert(_var_map.find(var_number) != _var_map.end(), "Variable does not exist.");
  if (_var_map[var_number].empty())
    return NULL;
  else
    return & _var_map[var_number];
}

void
SystemBase::addVariableToZeroOnResidual(std::string var_name)
{
  _vars_to_be_zeroed_on_residual.push_back(var_name);
}

void
SystemBase::addVariableToZeroOnJacobian(std::string var_name)
{
  _vars_to_be_zeroed_on_jacobian.push_back(var_name);
}

void
SystemBase::zeroVariables(std::vector<std::string> & vars_to_be_zeroed)
{
  if (vars_to_be_zeroed.size() > 0)
  {
    NumericVector<Number> & solution = this->solution();

    AllLocalDofIndicesThread aldit(system(), vars_to_be_zeroed);
    ConstElemRange & elem_range = *_mesh.getActiveLocalElementRange();
    Threads::parallel_reduce(elem_range, aldit);

    std::set<dof_id_type> dof_indices_to_zero = aldit._all_dof_indices;

    solution.close();

    for (const auto & dof : dof_indices_to_zero)
      solution.set(dof, 0);

    solution.close();

    // Call update to update the current_local_solution for this system
    system().update();
  }
}

void
SystemBase::zeroVariablesForResidual()
{
  zeroVariables(_vars_to_be_zeroed_on_residual);
}

void
SystemBase::zeroVariablesForJacobian()
{
  zeroVariables(_vars_to_be_zeroed_on_jacobian);
}

Order
SystemBase::getMinQuadratureOrder()
{
  Order order = CONSTANT;
  std::vector<MooseVariable *> vars = _vars[0].variables();
  for (const auto & var : vars)
  {
    FEType fe_type = var->feType();
    if (fe_type.default_quadrature_order() > order)
      order = fe_type.default_quadrature_order();
  }

  return order;
}

void
SystemBase::prepare(THREAD_ID tid)
{
  if (_subproblem.hasActiveElementalMooseVariables(tid))
  {
    const std::set<MooseVariable *> & active_elemental_moose_variables = _subproblem.getActiveElementalMooseVariables(tid);
    const std::vector<MooseVariable *> & vars = _vars[tid].variables();
    for (const auto & var : vars)
      var->clearDofIndices();

    for (const auto & var : active_elemental_moose_variables)
      if (&(var->sys()) == this)
        var->prepare();
  }
  else
  {
    const std::vector<MooseVariable *> & vars = _vars[tid].variables();
    for (const auto & var : vars)
      var->prepare();
  }
}

void
SystemBase::prepareFace(THREAD_ID tid, bool resize_data)
{
  if (_subproblem.hasActiveElementalMooseVariables(tid)) // We only need to do something if the element prepare was restricted
  {
    const std::set<MooseVariable *> & active_elemental_moose_variables = _subproblem.getActiveElementalMooseVariables(tid);

    std::vector<MooseVariable *> newly_prepared_vars;

    const std::vector<MooseVariable *> & vars = _vars[tid].variables();
    for (const auto & var : vars)
    {
      if (&(var->sys()) == this && !active_elemental_moose_variables.count(var)) // If it wasnt in the active list we need to prepare it
      {
        var->prepare();
        newly_prepared_vars.push_back(var);
      }
    }

    // Make sure to resize the residual and jacobian datastructures for all the new variables
    if (resize_data)
      for (unsigned int i=0; i<newly_prepared_vars.size(); i++)
      {
        _subproblem.assembly(tid).prepareVariable(newly_prepared_vars[i]);
        if (_subproblem.checkNonlocalCouplingRequirement())
          _subproblem.assembly(tid).prepareVariableNonlocal(newly_prepared_vars[i]);
      }
  }
}

void
SystemBase::prepareNeighbor(THREAD_ID tid)
{
  const std::vector<MooseVariable *> & vars = _vars[tid].variables();
  for (const auto & var : vars)
    var->prepareNeighbor();
}


void
SystemBase::reinitElem(const Elem * /*elem*/, THREAD_ID tid)
{

  if (_subproblem.hasActiveElementalMooseVariables(tid))
  {
    const std::set<MooseVariable *> & active_elemental_moose_variables = _subproblem.getActiveElementalMooseVariables(tid);
    for (const auto & var : active_elemental_moose_variables)
      if (&(var->sys()) == this)
        var->computeElemValues();
  }
  else
  {
    const std::vector<MooseVariable *> & vars = _vars[tid].variables();
    for (const auto & var : vars)
      var->computeElemValues();
  }
}

void
SystemBase::reinitElemFace(const Elem * /*elem*/, unsigned int /*side*/, BoundaryID /*bnd_id*/, THREAD_ID tid)
{
  const std::vector<MooseVariable *> & vars = _vars[tid].variables();
  for (const auto & var : vars)
    var->computeElemValuesFace();
}

void
SystemBase::reinitNeighborFace(const Elem * /*elem*/, unsigned int /*side*/, BoundaryID /*bnd_id*/, THREAD_ID tid)
{
  const std::vector<MooseVariable *> & vars = _vars[tid].variables();
  for (const auto & var : vars)
    var->computeNeighborValuesFace();
}

void
SystemBase::reinitNeighbor(const Elem * /*elem*/, THREAD_ID tid)
{
  const std::vector<MooseVariable *> & vars = _vars[tid].variables();
  for (const auto & var : vars)
    var->computeNeighborValues();
}

void
SystemBase::reinitNode(const Node * /*node*/, THREAD_ID tid)
{
  const std::vector<MooseVariable *> & vars = _vars[tid].variables();
  for (const auto & var : vars)
  {
    if (var->isNodal())
    {
      var->reinitNode();
      var->computeNodalValues();
    }
  }
}

void
SystemBase::reinitNodeFace(const Node * /*node*/, BoundaryID /*bnd_id*/, THREAD_ID tid)
{
  const std::vector<MooseVariable *> & vars = _vars[tid].variables();
  for (const auto & var : vars)
  {
    if (var->isNodal())
    {
      var->reinitNode();
      var->computeNodalValues();
    }
  }
}

void
SystemBase::reinitNodeNeighbor(const Node * /*node*/, THREAD_ID tid)
{
  const std::vector<MooseVariable *> & vars = _vars[tid].variables();
  for (const auto & var : vars)
  {
    if (var->isNodal())
    {
      var->reinitNodeNeighbor();
      var->computeNodalNeighborValues();
    }
  }
}

void
SystemBase::reinitNodes(const std::vector<dof_id_type> & nodes, THREAD_ID tid)
{
  const std::vector<MooseVariable *> & vars = _vars[tid].variables();
  for (const auto & var : vars)
  {
    var->reinitNodes(nodes);
    var->computeNodalValues();
  }
}

void
SystemBase::reinitNodesNeighbor(const std::vector<dof_id_type> & nodes, THREAD_ID tid)
{
  const std::vector<MooseVariable *> & vars = _vars[tid].variables();
  for (const auto & var : vars)
  {
    var->reinitNodesNeighbor(nodes);
    var->computeNodalNeighborValues();
  }
}

void
SystemBase::reinitScalars(THREAD_ID tid)
{
  const std::vector<MooseVariableScalar *> & vars = _vars[tid].scalars();
  for (const auto & var : vars)
    var->reinit();
}

void
SystemBase::augmentSendList(std::vector<dof_id_type> & send_list)
{
  std::set<dof_id_type> & ghosted_elems = _subproblem.ghostedElems();

  DofMap & dof_map = dofMap();

  std::vector<dof_id_type> dof_indices;

  System & sys = system();

  unsigned int sys_num = sys.number();

  unsigned int n_vars = sys.n_vars();

  for (const auto & elem_id : ghosted_elems)
  {
    Elem * elem = _mesh.elemPtr(elem_id);

    if (elem->active())
    {
      dof_map.dof_indices(elem, dof_indices);

      // Only need to ghost it if it's actually not on this processor
      for (const auto & dof : dof_indices)
        if (dof < dof_map.first_dof() || dof >= dof_map.end_dof())
          send_list.push_back(dof);

      // Now add the DoFs from all of the nodes.  This is necessary because of block
      // restricted variables.  A variable might not live _on_ this element but it
      // might live on nodes connected to this element.
      for (unsigned int n=0; n<elem->n_nodes(); n++)
      {
        Node * node = elem->node_ptr(n);

        // Have to get each variable's dofs
        for (unsigned int v=0; v<n_vars; v++)
        {
          const Variable & var = sys.variable(v);
          unsigned int var_num = var.number();
          unsigned int n_comp = var.n_components();

          // See if this variable has any dofs at this node
          if (node->n_dofs(sys_num, var_num) > 0)
          {
            // Loop over components of the variable
            for (unsigned int c=0; c<n_comp; c++)
              send_list.push_back(node->dof_number(sys_num, var_num, c));
          }
        }
      }
    }
  }
}
