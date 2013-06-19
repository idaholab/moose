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
// libMesh
#include "libmesh/quadrature_gauss.h"

/// Free function used for a libMesh callback
void extraSendList(std::vector<unsigned int> & send_list, void * context)
{
  SystemBase * sys = static_cast<SystemBase *>(context);
  sys->augmentSendList(send_list);
}

/// Free function used for a libMesh callback
void extraSparsity(SparsityPattern::Graph & sparsity,
                   std::vector<unsigned int> & n_nz,
                   std::vector<unsigned int> & n_oz,
                   void * context)
{
  SystemBase * sys = static_cast<SystemBase *>(context);
  sys->augmentSparsity(sparsity, n_nz, n_oz);
}

SystemBase::SystemBase(SubProblem & subproblem, const std::string & name) :
    _subproblem(subproblem),
    _app(subproblem.getMooseApp()),
    _factory(_app.getFactory()),
    _mesh(subproblem.mesh()),
    _name(name),
    _currently_computing_jacobian(false),
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
  if(vars_to_be_zeroed.size() > 0)
  {
    NumericVector<Number> & solution = this->solution();

    AllLocalDofIndicesThread aldit(system(), vars_to_be_zeroed);
    ConstElemRange & elem_range = *_mesh.getActiveLocalElementRange();
    Threads::parallel_reduce(elem_range, aldit);

    std::set<unsigned int> dof_indices_to_zero = aldit._all_dof_indices;

    solution.close();

    for(std::set<unsigned int>::iterator it = dof_indices_to_zero.begin();
        it != dof_indices_to_zero.end();
        ++it)
      solution.set(*it, 0);

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
  for (std::vector<MooseVariable *>::iterator it = vars.begin(); it != vars.end(); ++it)
  {
    FEType fe_type = (*it)->feType();
    if (fe_type.default_quadrature_order() > order)
      order = fe_type.default_quadrature_order();
  }

  return order;
}

void
SystemBase::prepare(THREAD_ID tid)
{
  if(_subproblem.hasActiveElementalMooseVariables(tid))
  {
    const std::set<MooseVariable *> & active_elemental_moose_variables = _subproblem.getActiveElementalMooseVariables(tid);
    const std::vector<MooseVariable *> & vars = _vars[tid].variables();
    for (std::vector<MooseVariable *>::const_iterator it = vars.begin(); it != vars.end(); ++it)
      (*it)->clearDofIndices();

    for(std::set<MooseVariable *>::iterator it = active_elemental_moose_variables.begin();
        it != active_elemental_moose_variables.end();
        ++it)
      if(&(*it)->sys() == this)
        (*it)->prepare();
  }
  else
  {
    const std::vector<MooseVariable *> & vars = _vars[tid].variables();
    for (std::vector<MooseVariable *>::const_iterator it = vars.begin(); it != vars.end(); ++it)
    {
      MooseVariable *var = *it;
      var->prepare();
    }
  }
}

void
SystemBase::prepareFace(THREAD_ID tid, bool resize_data)
{
  if(_subproblem.hasActiveElementalMooseVariables(tid)) // We only need to do something if the element prepare was restricted
  {
    const std::set<MooseVariable *> & active_elemental_moose_variables = _subproblem.getActiveElementalMooseVariables(tid);

    std::vector<MooseVariable *> newly_prepared_vars;

    const std::vector<MooseVariable *> & vars = _vars[tid].variables();
    for (std::vector<MooseVariable *>::const_iterator it = vars.begin(); it != vars.end(); ++it)
    {
      MooseVariable *var = *it;
      if(&var->sys() == this && !active_elemental_moose_variables.count(var)) // If it wasnt in the active list we need to prepare it
      {
        var->prepare();
        newly_prepared_vars.push_back(var);
      }
    }

    // Make sure to resize the residual and jacobian datastructures for all the new variables
    if(resize_data)
      for(unsigned int i=0; i<newly_prepared_vars.size(); i++)
        _subproblem.assembly(tid).prepareVariable(newly_prepared_vars[i]);
  }
}

void
SystemBase::prepareNeighbor(THREAD_ID tid)
{
  const std::vector<MooseVariable *> & vars = _vars[tid].variables();
  for (std::vector<MooseVariable *>::const_iterator it = vars.begin(); it != vars.end(); ++it)
  {
    MooseVariable *var = *it;
    var->prepareNeighbor();
  }
}


void
SystemBase::reinitElem(const Elem * /*elem*/, THREAD_ID tid)
{
  const std::set<MooseVariable *> & active_elemental_moose_variables = _subproblem.getActiveElementalMooseVariables(tid);

  if(_subproblem.hasActiveElementalMooseVariables(tid))
  {
    for(std::set<MooseVariable *>::iterator it = active_elemental_moose_variables.begin();
        it != active_elemental_moose_variables.end();
        ++it)
      if(&(*it)->sys() == this)
        (*it)->computeElemValues();
  }
  else
  {
    const std::vector<MooseVariable *> & vars = _vars[tid].variables();
    for (std::vector<MooseVariable *>::const_iterator it = vars.begin(); it != vars.end(); ++it)
    {
      MooseVariable *var = *it;
      var->computeElemValues();
    }
  }
}

void
SystemBase::reinitElemFace(const Elem * /*elem*/, unsigned int /*side*/, BoundaryID /*bnd_id*/, THREAD_ID tid)
{
  const std::vector<MooseVariable *> & vars = _vars[tid].variables();
  for (std::vector<MooseVariable *>::const_iterator it = vars.begin(); it != vars.end(); ++it)
  {
    MooseVariable *var = *it;
    var->computeElemValuesFace();
  }
}

void
SystemBase::reinitNeighborFace(const Elem * /*elem*/, unsigned int /*side*/, BoundaryID /*bnd_id*/, THREAD_ID tid)
{
  const std::vector<MooseVariable *> & vars = _vars[tid].variables();
  for (std::vector<MooseVariable *>::const_iterator it = vars.begin(); it != vars.end(); ++it)
  {
    MooseVariable *var = *it;
    var->computeNeighborValuesFace();
  }
}

void
SystemBase::reinitNeighbor(const Elem * /*elem*/, THREAD_ID tid)
{
  const std::vector<MooseVariable *> & vars = _vars[tid].variables();
  for (std::vector<MooseVariable *>::const_iterator it = vars.begin(); it != vars.end(); ++it)
  {
    MooseVariable *var = *it;
    var->computeNeighborValues();
  }
}

void
SystemBase::reinitNode(const Node * /*node*/, THREAD_ID tid)
{
  const std::vector<MooseVariable *> & vars = _vars[tid].variables();
  for (std::vector<MooseVariable *>::const_iterator it = vars.begin(); it != vars.end(); ++it)
  {
    MooseVariable *var = *it;
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
  for (std::vector<MooseVariable *>::const_iterator it = vars.begin(); it != vars.end(); ++it)
  {
    MooseVariable *var = *it;
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
  for (std::vector<MooseVariable *>::const_iterator it = vars.begin(); it != vars.end(); ++it)
  {
    MooseVariable *var = *it;
    if (var->isNodal())
    {
      var->reinitNodeNeighbor();
      var->computeNodalNeighborValues();
    }
  }
}

void
SystemBase::reinitNodes(const std::vector<unsigned int> & nodes, THREAD_ID tid)
{
  const std::vector<MooseVariable *> & vars = _vars[tid].variables();
  for (std::vector<MooseVariable *>::const_iterator it = vars.begin(); it != vars.end(); ++it)
  {
    MooseVariable *var = *it;
    var->reinitNodes(nodes);
    var->computeNodalValues();
  }
}

void
SystemBase::reinitScalars(THREAD_ID tid)
{
  const std::vector<MooseVariableScalar *> & vars = _vars[tid].scalars();
  for (std::vector<MooseVariableScalar *>::const_iterator it = vars.begin(); it != vars.end(); ++it)
  {
    MooseVariableScalar *var = *it;
    var->reinit();
  }
}


void
SystemBase::addInitialCondition(const std::string & ic_name, const std::string & name, InputParameters parameters)
{
  parameters.set<SubProblem *>("_subproblem") = &_subproblem;
  parameters.set<SystemBase *>("_sys") = this;

  const std::string & var_name = parameters.get<VariableName>("variable");
  const std::vector<SubdomainName> & blocks = parameters.get<std::vector<SubdomainName> >("block");

  for(unsigned int tid=0; tid < libMesh::n_threads(); tid++)
  {
    parameters.set<THREAD_ID>("_tid") = tid;
    if (blocks.size() > 0)
      for (unsigned int i = 0; i < blocks.size(); i++)
      {
        SubdomainID blk_id = _mesh.getSubdomainID(blocks[i]);
        _vars[tid].addInitialCondition(var_name, blk_id, static_cast<InitialCondition *>(_factory.create(ic_name, name, parameters)));
      }
    else
      _vars[tid].addInitialCondition(var_name, Moose::ANY_BLOCK_ID, static_cast<InitialCondition *>(_factory.create(ic_name, name, parameters)));
  }
}

void
SystemBase::addScalarInitialCondition(const std::string & ic_name, const std::string & name, InputParameters parameters)
{
  parameters.set<SubProblem *>("_subproblem") = &_subproblem;
  parameters.set<SystemBase *>("_sys") = this;

  const std::string & var_name = parameters.get<VariableName>("variable");

  for(unsigned int tid=0; tid < libMesh::n_threads(); tid++)
  {
    parameters.set<THREAD_ID>("_tid") = tid;
    _vars[tid].addScalarInitialCondition(var_name, static_cast<ScalarInitialCondition *>(_factory.create(ic_name, name, parameters)));
  }
}

void
SystemBase::projectSolution()
{
  if (system().n_dofs() <= 0)
    return;

  START_LOG("projectSolution()", "SystemTempl");

  ConstElemRange & elem_range = *_mesh.getActiveLocalElementRange();
  ComputeInitialConditionThread cic(_subproblem, *this, solution());
  //Threads::parallel_reduce(elem_range, cic);
  cic(elem_range);  // Disabled - Ticket #2012

  // Also, load values into the SCALAR dofs
  // Note: We assume that all SCALAR dofs are on the
  // processor with highest ID
  if(libMesh::processor_id() == (libMesh::n_processors()-1))
  {
    THREAD_ID tid = 0;
    const std::vector<MooseVariableScalar *> & scalar_vars = _vars[tid].scalars();
    for (std::vector<MooseVariableScalar *>::const_iterator it = scalar_vars.begin(); it != scalar_vars.end(); ++it)
    {
      MooseVariableScalar * var = *it;
      ScalarInitialCondition * sic = _vars[tid].getScalarInitialCondition(var->name());
      if (sic != NULL)
      {
        var->reinit();

        DenseVector<Number> vals(var->order());
        sic->compute(vals);

        const unsigned int n_SCALAR_dofs = var->dofIndices().size();
        for (unsigned int i = 0; i < n_SCALAR_dofs; i++)
        {
          const unsigned int global_index = var->dofIndices()[i];
          solution().set(global_index, vals(i));
        }
      }
    }
  }

  solution().close();

#ifdef LIBMESH_ENABLE_CONSTRAINTS
  dofMap().enforce_constraints_exactly(system(), &solution());
#endif

  STOP_LOG("projectSolution()", "SystemTempl");

  solution().localize(*system().current_local_solution, dofMap().get_send_list());
}
