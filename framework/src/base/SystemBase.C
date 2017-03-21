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
void
extraSendList(std::vector<dof_id_type> & send_list, void * context)
{
  SystemBase * sys = static_cast<SystemBase *>(context);
  sys->augmentSendList(send_list);
}

/// Free function used for a libMesh callback
void
extraSparsity(SparsityPattern::Graph & sparsity,
              std::vector<dof_id_type> & n_nz,
              std::vector<dof_id_type> & n_oz,
              void * context)
{
  SystemBase * sys = static_cast<SystemBase *>(context);
  sys->augmentSparsity(sparsity, n_nz, n_oz);
}

template <>
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

template <>
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

SystemBase::SystemBase(SubProblem & subproblem,
                       const std::string & name,
                       Moose::VarKindType var_kind)
  : libMesh::ParallelObject(subproblem),
    _subproblem(subproblem),
    _app(subproblem.getMooseApp()),
    _factory(_app.getFactory()),
    _mesh(subproblem.mesh()),
    _name(name),
    _vars(libMesh::n_threads()),
    _var_map(),
    _dummy_vec(NULL),
    _saved_old(NULL),
    _saved_older(NULL),
    _var_kind(var_kind)
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
  MooseVariableScalar * var =
      dynamic_cast<MooseVariableScalar *>(_vars[tid].getVariable(var_number));
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
    return &_var_map[var_number];
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
    const std::set<MooseVariable *> & active_elemental_moose_variables =
        _subproblem.getActiveElementalMooseVariables(tid);
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
  if (_subproblem.hasActiveElementalMooseVariables(
          tid)) // We only need to do something if the element prepare was restricted
  {
    const std::set<MooseVariable *> & active_elemental_moose_variables =
        _subproblem.getActiveElementalMooseVariables(tid);

    std::vector<MooseVariable *> newly_prepared_vars;

    const std::vector<MooseVariable *> & vars = _vars[tid].variables();
    for (const auto & var : vars)
    {
      if (&(var->sys()) == this &&
          !active_elemental_moose_variables.count(
              var)) // If it wasnt in the active list we need to prepare it
      {
        var->prepare();
        newly_prepared_vars.push_back(var);
      }
    }

    // Make sure to resize the residual and jacobian datastructures for all the new variables
    if (resize_data)
      for (unsigned int i = 0; i < newly_prepared_vars.size(); i++)
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
    const std::set<MooseVariable *> & active_elemental_moose_variables =
        _subproblem.getActiveElementalMooseVariables(tid);
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
SystemBase::reinitElemFace(const Elem * /*elem*/,
                           unsigned int /*side*/,
                           BoundaryID /*bnd_id*/,
                           THREAD_ID tid)
{
  const std::vector<MooseVariable *> & vars = _vars[tid].variables();
  for (const auto & var : vars)
    var->computeElemValuesFace();
}

void
SystemBase::reinitNeighborFace(const Elem * /*elem*/,
                               unsigned int /*side*/,
                               BoundaryID /*bnd_id*/,
                               THREAD_ID tid)
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
      for (unsigned int n = 0; n < elem->n_nodes(); n++)
      {
        Node * node = elem->node_ptr(n);

        // Have to get each variable's dofs
        for (unsigned int v = 0; v < n_vars; v++)
        {
          const Variable & var = sys.variable(v);
          unsigned int var_num = var.number();
          unsigned int n_comp = var.n_components();

          // See if this variable has any dofs at this node
          if (node->n_dofs(sys_num, var_num) > 0)
          {
            // Loop over components of the variable
            for (unsigned int c = 0; c < n_comp; c++)
              send_list.push_back(node->dof_number(sys_num, var_num, c));
          }
        }
      }
    }
  }
}

/**
 * Save the old and older solutions.
 */
void
SystemBase::saveOldSolutions()
{
  if (!_saved_old)
    _saved_old = &addVector("save_solution_old", false, PARALLEL);
  if (!_saved_older)
    _saved_older = &addVector("save_solution_older", false, PARALLEL);
  *_saved_old = solutionOld();
  *_saved_older = solutionOlder();
}

/**
 * Restore the old and older solutions when the saved solutions present.
 */
void
SystemBase::restoreOldSolutions()
{
  if (_saved_old)
  {
    solutionOld() = *_saved_old;
    removeVector("save_solution_old");
    _saved_old = NULL;
  }
  if (_saved_older)
  {
    solutionOlder() = *_saved_older;
    removeVector("save_solution_older");
    _saved_older = NULL;
  }
}

NumericVector<Number> &
SystemBase::addVector(const std::string & vector_name, const bool project, const ParallelType type)
{
  if (hasVector(vector_name))
    return getVector(vector_name);

  NumericVector<Number> * vec = &system().add_vector(vector_name, project, type);
  return *vec;
}

void
SystemBase::addVariable(const std::string & var_name,
                        const FEType & type,
                        Real scale_factor,
                        const std::set<SubdomainID> * const active_subdomains)
{
  unsigned int var_num = system().add_variable(var_name, type, active_subdomains);
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  {
    // FIXME: we cannot refer fetype in libMesh at this point, so we will just make a copy in
    // MooseVariableBase.
    MooseVariable * var =
        new MooseVariable(var_num, type, *this, _subproblem.assembly(tid), _var_kind);
    var->scalingFactor(scale_factor);
    _vars[tid].add(var_name, var);
  }
  if (active_subdomains == NULL)
    _var_map[var_num] = std::set<SubdomainID>();
  else
    for (std::set<SubdomainID>::iterator it = active_subdomains->begin();
         it != active_subdomains->end();
         ++it)
      _var_map[var_num].insert(*it);
}

void
SystemBase::addScalarVariable(const std::string & var_name,
                              Order order,
                              Real scale_factor,
                              const std::set<SubdomainID> * const active_subdomains)
{
  FEType type(order, SCALAR);
  unsigned int var_num = system().add_variable(var_name, type, active_subdomains);
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  {
    // FIXME: we cannot refer fetype in libMesh at this point, so we will just make a copy in
    // MooseVariableBase.
    MooseVariableScalar * var =
        new MooseVariableScalar(var_num, type, *this, _subproblem.assembly(tid), _var_kind);
    var->scalingFactor(scale_factor);
    _vars[tid].add(var_name, var);
  }
  if (active_subdomains == NULL)
    _var_map[var_num] = std::set<SubdomainID>();
  else
    for (std::set<SubdomainID>::iterator it = active_subdomains->begin();
         it != active_subdomains->end();
         ++it)
      _var_map[var_num].insert(*it);
}

bool
SystemBase::hasVariable(const std::string & var_name)
{
  if (system().has_variable(var_name))
    return system().variable_type(var_name).family != SCALAR;
  else
    return false;
}

bool
SystemBase::hasScalarVariable(const std::string & var_name)
{
  if (system().has_variable(var_name))
    return system().variable_type(var_name).family == SCALAR;
  else
    return false;
}

bool
SystemBase::isScalarVariable(unsigned int var_num)
{
  return (system().variable(var_num).type().family == SCALAR);
}

unsigned int
SystemBase::nVariables()
{
  return _vars[0].names().size();
}

/**
 * Check if the named vector exists in the system.
 */
bool
SystemBase::hasVector(const std::string & name)
{
  return system().have_vector(name);
}

/**
 * Get a raw NumericVector with the given name.
 */
NumericVector<Number> &
SystemBase::getVector(const std::string & name)
{
  return system().get_vector(name);
}

unsigned int
SystemBase::number()
{
  return system().number();
}

DofMap &
SystemBase::dofMap()
{
  return system().get_dof_map();
}

void
SystemBase::addVariableToCopy(const std::string & dest_name,
                              const std::string & source_name,
                              const std::string & timestep)
{
  _var_to_copy.push_back(VarCopyInfo(dest_name, source_name, timestep));
}

void
SystemBase::copyVars(ExodusII_IO & io)
{
  int n_steps = io.get_num_time_steps();

  bool did_copy = false;
  for (std::vector<VarCopyInfo>::iterator it = _var_to_copy.begin(); it != _var_to_copy.end(); ++it)
  {
    VarCopyInfo & vci = *it;
    int timestep = -1;

    if (vci._timestep == "LATEST")
      // Use the last time step in the file from which to retrieve the solution
      timestep = n_steps;
    else
    {
      std::istringstream ss(vci._timestep);
      if (!(ss >> timestep) || timestep > n_steps)
        mooseError("Invalid value passed as \"initial_from_file_timestep\". Expected \"LATEST\" or "
                   "a valid integer between 1 and ",
                   n_steps,
                   " inclusive, received ",
                   vci._timestep);
    }

    did_copy = true;
    if (getVariable(0, vci._dest_name).isNodal())
      io.copy_nodal_solution(system(), vci._dest_name, vci._source_name, timestep);
    else
      io.copy_elemental_solution(system(), vci._dest_name, vci._source_name, timestep);
  }

  if (did_copy)
    solution().close();
}

void
SystemBase::update()
{
  system().update();
}

void
SystemBase::solve()
{
  system().solve();
}

/**
 * Copy current solution into old and older
 */
void
SystemBase::copySolutionsBackwards()
{
  system().update();
  solutionOlder() = *currentSolution();
  solutionOld() = *currentSolution();
  if (solutionPreviousNewton())
    *solutionPreviousNewton() = *currentSolution();
}

/**
 * Shifts the solutions backwards in time
 */
void
SystemBase::copyOldSolutions()
{
  solutionOlder() = solutionOld();
  solutionOld() = *currentSolution();
  if (solutionPreviousNewton())
    *solutionPreviousNewton() = *currentSolution();
}

/**
 * Restore current solutions (call after your solve failed)
 */
void
SystemBase::restoreSolutions()
{
  *(const_cast<NumericVector<Number> *&>(currentSolution())) = solutionOld();
  solution() = solutionOld();
  if (solutionPreviousNewton())
    *solutionPreviousNewton() = solutionOld();
  system().update();
}
