//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseApp.h"
#include "SystemBase.h"
#include "Factory.h"
#include "SubProblem.h"
#include "MooseVariableFE.h"
#include "MooseVariableFV.h"
#include "MooseVariableScalar.h"
#include "MooseVariableConstMonomial.h"
#include "Conversion.h"
#include "Parser.h"
#include "AllLocalDofIndicesThread.h"
#include "MooseTypes.h"
#include "InitialCondition.h"
#include "ScalarInitialCondition.h"
#include "Assembly.h"
#include "MooseMesh.h"
#include "MooseUtils.h"
#include "FVBoundaryCondition.h"
#include "FVDirichletBC.h"

#include "libmesh/dof_map.h"
#include "libmesh/string_to_enum.h"

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

  // Need an l-value reference to pass to dataStore
  unsigned int num_vectors = libmesh_system.n_vectors();
  dataStore(stream, num_vectors, context);

  for (System::vectors_iterator it = libmesh_system.vectors_begin();
       it != libmesh_system.vectors_end();
       it++)
  {
    // Store the vector name. A map iterator will have a const Key, so we need to make a copy
    // because dataStore expects a non-const reference
    auto vector_name = it->first;
    dataStore(stream, vector_name, context);

    // Store the vector
    dataStore(stream, *(it->second), context);
  }
}

template <>
void
dataLoad(std::istream & stream, SystemBase & system_base, void * context)
{
  System & libmesh_system = system_base.system();

  NumericVector<Real> & solution = *(libmesh_system.solution.get());

  dataLoad(stream, solution, context);

  unsigned int num_vectors;
  dataLoad(stream, num_vectors, context);

  // Can't do a range based for loop because we don't actually use the index, resulting in an unused
  // variable warning. So we make a dumb index variable and use it in the loop termination check
  for (unsigned int vec_num = 0; vec_num < num_vectors; ++vec_num)
  {
    std::string vector_name;
    dataLoad(stream, vector_name, context);

    if (!libmesh_system.have_vector(vector_name))
      mooseError("Trying to load vector name ",
                 vector_name,
                 " but that vector doesn't exist in the system.");

    auto & vector = libmesh_system.get_vector(vector_name);

    dataLoad(stream, vector, context);
  }

  system_base.update();
}

SystemBase::SystemBase(SubProblem & subproblem,
                       const std::string & name,
                       Moose::VarKindType var_kind)
  : libMesh::ParallelObject(subproblem),
    ConsoleStreamInterface(subproblem.getMooseApp()),
    _subproblem(subproblem),
    _app(subproblem.getMooseApp()),
    _factory(_app.getFactory()),
    _mesh(subproblem.mesh()),
    _name(name),
    _vars(libMesh::n_threads()),
    _var_map(),
    _max_var_number(0),
    _saved_old(NULL),
    _saved_older(NULL),
    _saved_dot_old(NULL),
    _saved_dotdot_old(NULL),
    _var_kind(var_kind),
    _max_var_n_dofs_per_elem(0),
    _max_var_n_dofs_per_node(0),
    _time_integrator(nullptr),
    _computing_scaling_jacobian(false),
    _computing_scaling_residual(false),
    _automatic_scaling(false),
    _verbose(false),
    _default_solution_states(2)
{
}

MooseVariableFEBase &
SystemBase::getVariable(THREAD_ID tid, const std::string & var_name)
{
  MooseVariableFEBase * var = dynamic_cast<MooseVariableFEBase *>(_vars[tid].getVariable(var_name));
  if (!var)
    mooseError("Variable '", var_name, "' does not exist in this system");
  return *var;
}

MooseVariableFEBase &
SystemBase::getVariable(THREAD_ID tid, unsigned int var_number)
{
  if (var_number < _numbered_vars[tid].size())
    if (_numbered_vars[tid][var_number])
      return *_numbered_vars[tid][var_number];

  mooseError("Variable #", Moose::stringify(var_number), " does not exist in this system");
}

template <typename T>
MooseVariableFE<T> &
SystemBase::getFieldVariable(THREAD_ID tid, const std::string & var_name)
{
  return *_vars[tid].getFieldVariable<T>(var_name);
}

template <typename T>
MooseVariableField<T> &
SystemBase::getActualFieldVariable(THREAD_ID tid, const std::string & var_name)
{
  return *_vars[tid].getActualFieldVariable<T>(var_name);
}

template <typename T>
MooseVariableFE<T> &
SystemBase::getFieldVariable(THREAD_ID tid, unsigned int var_number)
{
  return *_vars[tid].getFieldVariable<T>(var_number);
}

template <typename T>
MooseVariableField<T> &
SystemBase::getActualFieldVariable(THREAD_ID tid, unsigned int var_number)
{
  return *_vars[tid].getActualFieldVariable<T>(var_number);
}

MooseVariableScalar &
SystemBase::getScalarVariable(THREAD_ID tid, const std::string & var_name)
{
  MooseVariableScalar * var = dynamic_cast<MooseVariableScalar *>(_vars[tid].getVariable(var_name));
  if (!var)
    mooseError("Scalar variable '" + var_name + "' does not exist in this system");
  return *var;
}

MooseVariableScalar &
SystemBase::getScalarVariable(THREAD_ID tid, unsigned int var_number)
{
  MooseVariableScalar * var =
      dynamic_cast<MooseVariableScalar *>(_vars[tid].getVariable(var_number));
  if (!var)
    mooseError("variable #" + Moose::stringify(var_number) + " does not exist in this system");
  return *var;
}

const std::set<SubdomainID> *
SystemBase::getVariableBlocks(unsigned int var_number)
{
  mooseAssert(_var_map.find(var_number) != _var_map.end(), "Variable does not exist.");
  if (_var_map[var_number].empty())
    return nullptr;
  else
    return &_var_map[var_number];
}

void
SystemBase::addVariableToZeroOnResidual(std::string var_name)
{
  unsigned int ncomp = getVariable(0, var_name).count();
  if (ncomp > 1)
    // need to push libMesh variable names for all components
    for (unsigned int i = 0; i < ncomp; ++i)
      _vars_to_be_zeroed_on_residual.push_back(_subproblem.arrayVariableComponent(var_name, i));
  else
    _vars_to_be_zeroed_on_residual.push_back(var_name);
}

void
SystemBase::addVariableToZeroOnJacobian(std::string var_name)
{
  unsigned int ncomp = getVariable(0, var_name).count();
  if (ncomp > 1)
    // need to push libMesh variable names for all components
    for (unsigned int i = 0; i < ncomp; ++i)
      _vars_to_be_zeroed_on_jacobian.push_back(_subproblem.arrayVariableComponent(var_name, i));
  else
    _vars_to_be_zeroed_on_jacobian.push_back(var_name);
}

void
SystemBase::zeroVariables(std::vector<std::string> & vars_to_be_zeroed)
{
  if (vars_to_be_zeroed.size() > 0)
  {
    NumericVector<Number> & solution = this->solution();

    AllLocalDofIndicesThread aldit(system(), vars_to_be_zeroed, true);
    ConstElemRange & elem_range = *_mesh.getActiveLocalElementRange();
    Threads::parallel_reduce(elem_range, aldit);

    const auto & dof_indices_to_zero = aldit.getDofIndices();

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
  const std::vector<MooseVariableFEBase *> & vars = _vars[0].fieldVariables();
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
    const std::set<MooseVariableFEBase *> & active_elemental_moose_variables =
        _subproblem.getActiveElementalMooseVariables(tid);
    const std::vector<MooseVariableFEBase *> & vars = _vars[tid].fieldVariables();
    for (const auto & var : vars)
      var->clearDofIndices();

    for (const auto & var : active_elemental_moose_variables)
      if (&(var->sys()) == this)
        var->prepare();
  }
  else
  {
    const std::vector<MooseVariableFEBase *> & vars = _vars[tid].fieldVariables();
    for (const auto & var : vars)
      var->prepare();
  }
}

void
SystemBase::prepareFace(THREAD_ID tid, bool resize_data)
{
  // We only need to do something if the element prepare was restricted
  if (_subproblem.hasActiveElementalMooseVariables(tid))
  {
    const std::set<MooseVariableFEBase *> & active_elemental_moose_variables =
        _subproblem.getActiveElementalMooseVariables(tid);

    std::vector<MooseVariableFEBase *> newly_prepared_vars;

    const std::vector<MooseVariableFEBase *> & vars = _vars[tid].fieldVariables();
    for (const auto & var : vars)
    {
      // If it wasn't in the active list, we need to prepare it
      if (&(var->sys()) == this && !active_elemental_moose_variables.count(var))
      {
        var->prepare();
        newly_prepared_vars.push_back(var);
      }
    }

    // Make sure to resize the residual and jacobian datastructures for all the new variables
    if (resize_data)
      for (const auto var_ptr : newly_prepared_vars)
      {
        _subproblem.assembly(tid).prepareVariable(var_ptr);
        if (_subproblem.checkNonlocalCouplingRequirement())
          _subproblem.assembly(tid).prepareVariableNonlocal(var_ptr);
      }
  }
}

void
SystemBase::prepareNeighbor(THREAD_ID tid)
{
  const std::vector<MooseVariableFEBase *> & vars = _vars[tid].fieldVariables();
  for (const auto & var : vars)
    var->prepareNeighbor();
}

void
SystemBase::prepareLowerD(THREAD_ID tid)
{
  const std::vector<MooseVariableFEBase *> & vars = _vars[tid].fieldVariables();
  for (const auto & var : vars)
    var->prepareLowerD();
}

void
SystemBase::reinitElem(const Elem * /*elem*/, THREAD_ID tid)
{

  if (_subproblem.hasActiveElementalMooseVariables(tid))
  {
    const std::set<MooseVariableFEBase *> & active_elemental_moose_variables =
        _subproblem.getActiveElementalMooseVariables(tid);
    for (const auto & var : active_elemental_moose_variables)
      if (&(var->sys()) == this)
        var->computeElemValues();
  }
  else
  {
    const std::vector<MooseVariableFEBase *> & vars = _vars[tid].fieldVariables();
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
  const std::vector<MooseVariableFEBase *> & vars = _vars[tid].fieldVariables();
  for (const auto & var : vars)
    var->computeElemValuesFace();
}

void
SystemBase::reinitNeighborFace(const Elem * /*elem*/,
                               unsigned int /*side*/,
                               BoundaryID /*bnd_id*/,
                               THREAD_ID tid)
{
  const std::vector<MooseVariableFEBase *> & vars = _vars[tid].fieldVariables();
  for (const auto & var : vars)
    var->computeNeighborValuesFace();
}

void
SystemBase::reinitNeighbor(const Elem * /*elem*/, THREAD_ID tid)
{
  const std::vector<MooseVariableFEBase *> & vars = _vars[tid].fieldVariables();
  for (const auto & var : vars)
    var->computeNeighborValues();
}

void
SystemBase::reinitLowerD(THREAD_ID tid)
{
  const std::vector<MooseVariableFEBase *> & vars = _vars[tid].fieldVariables();
  for (const auto & var : vars)
    var->computeLowerDValues();
}

void
SystemBase::reinitNode(const Node * /*node*/, THREAD_ID tid)
{
  const std::vector<MooseVariableFEBase *> & vars = _vars[tid].fieldVariables();
  for (const auto & var : vars)
  {
    var->reinitNode();
    if (var->isNodalDefined())
      var->computeNodalValues();
  }
}

void
SystemBase::reinitNodeFace(const Node * /*node*/, BoundaryID /*bnd_id*/, THREAD_ID tid)
{
  const std::vector<MooseVariableFEBase *> & vars = _vars[tid].fieldVariables();
  for (const auto & var : vars)
  {
    var->reinitNode();
    if (var->isNodalDefined())
      var->computeNodalValues();
  }
}

void
SystemBase::reinitNodes(const std::vector<dof_id_type> & nodes, THREAD_ID tid)
{
  const std::vector<MooseVariableFEBase *> & vars = _vars[tid].fieldVariables();
  for (const auto & var : vars)
  {
    var->reinitNodes(nodes);
    var->computeNodalValues();
  }
}

void
SystemBase::reinitNodesNeighbor(const std::vector<dof_id_type> & nodes, THREAD_ID tid)
{
  const std::vector<MooseVariableFEBase *> & vars = _vars[tid].fieldVariables();
  for (const auto & var : vars)
  {
    var->reinitNodesNeighbor(nodes);
    var->computeNodalNeighborValues();
  }
}

void
SystemBase::reinitScalars(THREAD_ID tid, bool reinit_for_derivative_reordering /*=false*/)
{
  const std::vector<MooseVariableScalar *> & vars = _vars[tid].scalars();
  for (const auto & var : vars)
    var->reinit(reinit_for_derivative_reordering);
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
  const auto states = _solution_states.size();
  if (states > 1)
  {
    _saved_solution_states.resize(states);
    for (unsigned int i = 1; i <= states - 1; ++i)
      if (!_saved_solution_states[i])
        _saved_solution_states[i] =
            &addVector("save_solution_state_" + std::to_string(i), false, PARALLEL);

    for (unsigned int i = 1; i <= states - 1; ++i)
      *(_saved_solution_states[i]) = solutionState(i);
  }

  if (!_saved_dot_old && solutionUDotOld())
    _saved_dot_old = &addVector("save_solution_dot_old", false, PARALLEL);
  if (!_saved_dotdot_old && solutionUDotDotOld())
    _saved_dotdot_old = &addVector("save_solution_dotdot_old", false, PARALLEL);

  if (solutionUDotOld())
    *_saved_dot_old = *solutionUDotOld();

  if (solutionUDotDotOld())
    *_saved_dotdot_old = *solutionUDotDotOld();
}

/**
 * Restore the old and older solutions when the saved solutions present.
 */
void
SystemBase::restoreOldSolutions()
{
  const auto states = _solution_states.size();
  if (states > 1)
    for (unsigned int i = 1; i <= states - 1; ++i)
      if (_saved_solution_states[i])
      {
        solutionState(i) = *(_saved_solution_states[i]);
        removeVector("save_solution_state_" + std::to_string(i));
        _saved_solution_states[i] = nullptr;
      }

  if (_saved_dot_old && solutionUDotOld())
  {
    *solutionUDotOld() = *_saved_dot_old;
    removeVector("save_solution_dot_old");
    _saved_dot_old = nullptr;
  }
  if (_saved_dotdot_old && solutionUDotDotOld())
  {
    *solutionUDotDotOld() = *_saved_dotdot_old;
    removeVector("save_solution_dotdot_old");
    _saved_dotdot_old = nullptr;
  }
}

NumericVector<Number> &
SystemBase::addVector(const std::string & vector_name, const bool project, const ParallelType type)
{
  if (hasVector(vector_name))
    return getVector(vector_name);

  NumericVector<Number> & vec = system().add_vector(vector_name, project, type);
  return vec;
}

NumericVector<Number> &
SystemBase::addVector(TagID tag, const bool project, const ParallelType type)
{
  if (!_subproblem.vectorTagExists(tag))
    mooseError("Cannot add a tagged vector with vector_tag, ",
               tag,
               ", that tag does not exist in System ",
               name());

  if (hasVector(tag))
    return getVector(tag);

  auto vector_name = _subproblem.vectorTagName(tag);

  NumericVector<Number> & vec = system().add_vector(vector_name, project, type);

  if (_tagged_vectors.size() < tag + 1)
    _tagged_vectors.resize(tag + 1);

  _tagged_vectors[tag] = &vec;

  return vec;
}

void
SystemBase::closeTaggedVectors(const std::set<TagID> & tags)
{
  for (auto & tag : tags)
  {
    mooseAssert(_subproblem.vectorTagExists(tag), "Tag: " << tag << " does not exsit");
    getVector(tag).close();
  }
}

void
SystemBase::zeroTaggedVectors(const std::set<TagID> & tags)
{
  for (auto & tag : tags)
  {
    mooseAssert(_subproblem.vectorTagExists(tag), "Tag: " << tag << " does not exsit");
    getVector(tag).zero();
  }
}

void
SystemBase::removeVector(TagID tag_id)
{
  if (!_subproblem.vectorTagExists(tag_id))
    mooseError("Cannot remove an unexisting tag or its associated vector, ",
               tag_id,
               ", that tag does not exist in System ",
               name());

  if (hasVector(tag_id))
  {
    auto vector_name = _subproblem.vectorTagName(tag_id);
    system().remove_vector(vector_name);
    _tagged_vectors[tag_id] = nullptr;
  }
}

void
SystemBase::addVariable(const std::string & var_type,
                        const std::string & name,
                        InputParameters & parameters)
{
  _numbered_vars.resize(libMesh::n_threads());

  auto components = parameters.get<unsigned int>("components");

  // Convert the std::vector parameter provided by the user into a std::set for use by libMesh's
  // System::add_variable method
  std::set<SubdomainID> blocks;
  const auto & block_param = parameters.get<std::vector<SubdomainName>>("block");
  for (const auto & subdomain_name : block_param)
  {
    SubdomainID blk_id = _mesh.getSubdomainID(subdomain_name);
    blocks.insert(blk_id);
  }

  auto fe_type = FEType(Utility::string_to_enum<Order>(parameters.get<MooseEnum>("order")),
                        Utility::string_to_enum<FEFamily>(parameters.get<MooseEnum>("family")));

  unsigned int var_num;

  if (var_type == "ArrayMooseVariable")
  {
    if (fe_type.family == NEDELEC_ONE || fe_type.family == LAGRANGE_VEC ||
        fe_type.family == MONOMIAL_VEC)
      mooseError("Vector family type cannot be used in an array variable");

    // Build up the variable names
    std::vector<std::string> var_names;
    for (unsigned int i = 0; i < components; i++)
      var_names.push_back(SubProblem::arrayVariableComponent(name, i));

    // The number returned by libMesh is the _last_ variable number... we want to hold onto the
    // _first_
    var_num = system().add_variables(var_names, fe_type, &blocks) - (components - 1);
  }
  else
    var_num = system().add_variable(name, fe_type, &blocks);

  parameters.set<unsigned int>("_var_num") = var_num;
  parameters.set<SystemBase *>("_system_base") = this;

  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  {
    parameters.set<THREAD_ID>("tid") = tid;
    std::shared_ptr<MooseVariableBase> var =
        _factory.create<MooseVariableBase>(var_type, name, parameters, tid);

    _vars[tid].add(name, var);

    if (auto fe_var = dynamic_cast<MooseVariableFEBase *>(var.get()))
    {
      auto required_size = var_num + components;
      if (required_size > _numbered_vars[tid].size())
        _numbered_vars[tid].resize(required_size);
      for (MooseIndex(components) component = 0; component < components; ++component)
        _numbered_vars[tid][var_num + component] = fe_var;
    }

    if (var->blockRestricted())
      for (const SubdomainID & id : var->blockIDs())
        for (MooseIndex(components) component = 0; component < components; ++component)
          _var_map[var_num + component].insert(id);
    else
      for (MooseIndex(components) component = 0; component < components; ++component)
        _var_map[var_num + component] = std::set<SubdomainID>();
  }

  // getMaxVariableNumber is an API method used in Rattlesnake
  if (var_num > _max_var_number)
    _max_var_number = var_num;
}

bool
SystemBase::hasVariable(const std::string & var_name) const
{
  auto & names = getVariableNames();
  if (system().has_variable(var_name))
    return system().variable_type(var_name).family != SCALAR;
  if (std::find(names.begin(), names.end(), var_name) != names.end())
    // array variable
    return true;
  else
    return false;
}

bool
SystemBase::isArrayVariable(const std::string & var_name) const
{
  auto & names = getVariableNames();
  if (!system().has_variable(var_name) &&
      std::find(names.begin(), names.end(), var_name) != names.end())
    // array variable
    return true;
  else
    return false;
}

bool
SystemBase::hasScalarVariable(const std::string & var_name) const
{
  if (system().has_variable(var_name))
    return system().variable_type(var_name).family == SCALAR;
  else
    return false;
}

bool
SystemBase::isScalarVariable(unsigned int var_num) const
{
  return (system().variable(var_num).type().family == SCALAR);
}

unsigned int
SystemBase::nVariables() const
{
  unsigned int n = 0;
  for (auto & var : _vars[0].fieldVariables())
    n += var->count();
  n += _vars[0].scalars().size();

  return n;
}

/**
 * Check if the named vector exists in the system.
 */
bool
SystemBase::hasVector(const std::string & name) const
{
  return system().have_vector(name);
}

TagID
SystemBase::timeVectorTag()
{
  mooseError("Not implemented yet");
  return 0;
}

TagID
SystemBase::timeMatrixTag()
{
  mooseError("Not implemented yet");
  return 0;
}

TagID
SystemBase::systemMatrixTag()
{
  mooseError("Not implemented yet");
  return 0;
}

TagID
SystemBase::nonTimeVectorTag()
{
  mooseError("Not implemented yet");
  return 0;
}

TagID
SystemBase::residualVectorTag()
{
  mooseError("Not implemented yet");
  return 0;
}

bool
SystemBase::hasVector(TagID tag) const
{
  return tag < _tagged_vectors.size() && _tagged_vectors[tag];
}

/**
 * Get a raw NumericVector with the given name.
 */
NumericVector<Number> &
SystemBase::getVector(const std::string & name)
{
  return system().get_vector(name);
}

NumericVector<Number> &
SystemBase::getVector(TagID tag)
{
  mooseAssert(hasVector(tag), "Cannot retrieve vector with residual_tag: " << tag);

  return *_tagged_vectors[tag];
}

const NumericVector<Number> &
SystemBase::getVector(TagID tag) const
{
  mooseAssert(hasVector(tag), "Cannot retrieve vector with residual_tag: " << tag);

  return *_tagged_vectors[tag];
}

void
SystemBase::associateVectorToTag(NumericVector<Number> & vec, TagID tag)
{
  mooseAssert(_subproblem.vectorTagExists(tag),
              "You can't associate a tag that does not exist " << tag);
  if (_tagged_vectors.size() < tag + 1)
    _tagged_vectors.resize(tag + 1);

  _tagged_vectors[tag] = &vec;
}

void
SystemBase::disassociateVectorFromTag(NumericVector<Number> & vec, TagID tag)
{
  mooseAssert(_subproblem.vectorTagExists(tag),
              "You can't associate a tag that does not exist " << tag);
  if (_tagged_vectors.size() < tag + 1)
    _tagged_vectors.resize(tag + 1);

  if (_tagged_vectors[tag] != &vec)
    mooseError("You can not disassociate a vector from a tag which it was not associated to");

  _tagged_vectors[tag] = nullptr;
}

void
SystemBase::disassociateAllTaggedVectors()
{
  for (auto & tagged_vector : _tagged_vectors)
    tagged_vector = nullptr;
}

bool
SystemBase::hasMatrix(TagID tag) const
{
  return tag < _tagged_matrices.size() && _tagged_matrices[tag];
}

SparseMatrix<Number> &
SystemBase::getMatrix(TagID tag)
{
  mooseAssert(hasMatrix(tag), "Cannot retrieve matrix with matrix_tag: " << tag);

  return *_tagged_matrices[tag];
}

const SparseMatrix<Number> &
SystemBase::getMatrix(TagID tag) const
{
  mooseAssert(hasMatrix(tag), "Cannot retrieve matrix with matrix_tag: " << tag);

  return *_tagged_matrices[tag];
}

void
SystemBase::closeTaggedMatrices(const std::set<TagID> & tags)
{
  for (auto tag : tags)
    if (hasMatrix(tag))
      getMatrix(tag).close();
}

void
SystemBase::associateMatrixToTag(SparseMatrix<Number> & matrix, TagID tag)
{
  mooseAssert(_subproblem.matrixTagExists(tag),
              "Cannot associate Matrix with matrix_tag : " << tag << "that does not exist");

  if (_tagged_matrices.size() < tag + 1)
    _tagged_matrices.resize(tag + 1);

  _tagged_matrices[tag] = &matrix;
}

void
SystemBase::disassociateMatrixFromTag(SparseMatrix<Number> & matrix, TagID tag)
{
  mooseAssert(_subproblem.matrixTagExists(tag),
              "Cannot disassociate Matrix with matrix_tag : " << tag << "that does not exist");

  if (_tagged_matrices.size() < tag + 1)
    _tagged_matrices.resize(tag + 1);

  if (_tagged_matrices[tag] != &matrix)
    mooseError("You can not disassociate a matrix from a tag which it was not associated to");

  _tagged_matrices[tag] = nullptr;
}

void
SystemBase::activeMatrixTag(TagID tag)
{
  mooseAssert(_subproblem.matrixTagExists(tag),
              "Cannot active Matrix with matrix_tag : " << tag << "that does not exist");

  if (_matrix_tag_active_flags.size() < tag + 1)
    _matrix_tag_active_flags.resize(tag + 1);

  _matrix_tag_active_flags[tag] = true;
}

void
SystemBase::deactiveMatrixTag(TagID tag)
{
  mooseAssert(_subproblem.matrixTagExists(tag),
              "Cannot deactivate Matrix with matrix_tag : " << tag << "that does not exist");

  if (_matrix_tag_active_flags.size() < tag + 1)
    _matrix_tag_active_flags.resize(tag + 1);

  _matrix_tag_active_flags[tag] = false;
}

void
SystemBase::deactiveAllMatrixTags()
{
  auto num_matrix_tags = _subproblem.numMatrixTags();

  _matrix_tag_active_flags.resize(num_matrix_tags);

  for (decltype(num_matrix_tags) tag = 0; tag < num_matrix_tags; tag++)
    _matrix_tag_active_flags[tag] = false;
}

void
SystemBase::activeAllMatrixTags()
{
  auto num_matrix_tags = _subproblem.numMatrixTags();

  _matrix_tag_active_flags.resize(num_matrix_tags);

  for (decltype(num_matrix_tags) tag = 0; tag < num_matrix_tags; tag++)
    if (hasMatrix(tag))
      _matrix_tag_active_flags[tag] = true;
    else
      _matrix_tag_active_flags[tag] = false;
}

bool
SystemBase::matrixTagActive(TagID tag) const
{
  mooseAssert(_subproblem.matrixTagExists(tag), "Matrix tag " << tag << " does not exist");

  return tag < _matrix_tag_active_flags.size() && _matrix_tag_active_flags[tag];
}

void
SystemBase::disassociateAllTaggedMatrices()
{
  for (auto & matrix : _tagged_matrices)
    matrix = nullptr;
}

unsigned int
SystemBase::number() const
{
  return system().number();
}

DofMap &
SystemBase::dofMap()
{
  return system().get_dof_map();
}

const DofMap &
SystemBase::dofMap() const
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
  for (const auto & vci : _var_to_copy)
  {
    int timestep = -1;

    if (vci._timestep == "LATEST")
      // Use the last time step in the file from which to retrieve the solution
      timestep = n_steps;
    else
    {
      timestep = MooseUtils::convert<int>(vci._timestep);
      if (timestep > n_steps)
        mooseError("Invalid value passed as \"initial_from_file_timestep\". Expected \"LATEST\" or "
                   "a valid integer between 1 and ",
                   n_steps,
                   " inclusive, received ",
                   vci._timestep);
    }

    did_copy = true;

    if (hasVariable(vci._dest_name))
    {
      if (getVariable(0, vci._dest_name).isNodal())
        io.copy_nodal_solution(system(), vci._dest_name, vci._source_name, timestep);

      else
        io.copy_elemental_solution(system(), vci._dest_name, vci._source_name, timestep);
    }
    else if (hasScalarVariable(vci._dest_name))
    {
      auto rank = comm().rank();
      auto size = comm().size();

      // Read solution on rank 0 only and send data to rank "size - 1" where scalar DOFs are
      // stored
      std::vector<Real> global_values;
      if (rank == 0)
      {
        // Read the scalar value then set that value in the current solution
        io.read_global_variable({vci._source_name}, timestep, global_values);
        if (size > 1)
          comm().send(size - 1, global_values);
      }
      if (rank == size - 1)
      {
        if (size > 1)
          comm().receive(0, global_values);
        const unsigned int var_num = system().variable_number(vci._dest_name);
        system().solution->set(var_num, global_values[0]);
      }
    }
  }

  if (did_copy)
    solution().close();
}

void
SystemBase::addExtraVectors()
{
}

void
SystemBase::update()
{
  system().update();
  std::vector<VariableName> std_field_variables;
  getStandardFieldVariableNames(std_field_variables);
  cacheVarIndicesByFace(std_field_variables);
}

void
SystemBase::solve()
{
  system().solve();
}

void
SystemBase::getStandardFieldVariableNames(std::vector<VariableName> & std_field_variables) const
{
  std_field_variables.clear();
  for (auto & p : _vars[0].fieldVariables())
    if (p->fieldType() == 0)
      std_field_variables.push_back(p->name());
}

/**
 * Copy current solution into old and older
 */
void
SystemBase::copySolutionsBackwards()
{
  system().update();

  const auto states = _solution_states.size();
  if (states > 1)
    for (unsigned int i = 1; i <= states - 1; ++i)
      solutionState(i) = solutionState(0);

  if (solutionUDotOld())
    *solutionUDotOld() = *solutionUDot();
  if (solutionUDotDotOld())
    *solutionUDotDotOld() = *solutionUDotDot();
  if (solutionPreviousNewton())
    *solutionPreviousNewton() = *currentSolution();
}

/**
 * Shifts the solutions backwards in time
 */
void
SystemBase::copyOldSolutions()
{
  const auto states = _solution_states.size();
  if (states > 1)
    for (unsigned int i = states - 1; i > 0; --i)
      solutionState(i) = solutionState(i - 1);

  if (solutionUDotOld())
    *solutionUDotOld() = *solutionUDot();
  if (solutionUDotDotOld())
    *solutionUDotDotOld() = *solutionUDotDot();
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
  if (solutionUDotOld())
    *solutionUDot() = *solutionUDotOld();
  if (solutionUDotDotOld())
    *solutionUDotDot() = *solutionUDotDotOld();
  if (solutionPreviousNewton())
    *solutionPreviousNewton() = solutionOld();
  system().update();
}

void
SystemBase::removeVector(const std::string & name)
{
  system().remove_vector(name);
}

const std::string &
SystemBase::name() const
{
  return system().name();
}

const NumericVector<Number> &
SystemBase::solutionState(const unsigned int state) const
{
  mooseAssert(
      !_solution_states.empty(),
      "No solution states available: make sure to init the default states in system constructors");

  if (state >= _solution_states.size())
    mooseError("Solution state ",
               state,
               " was requested in ",
               name(),
               " but only up to state ",
               _solution_states.size() - 1,
               " is available.");

  return *_solution_states[state];
}

NumericVector<Number> &
SystemBase::solutionState(const unsigned int state)
{
  // Create up to the state requested if unavailable
  if (state >= _solution_states.size())
  {
    _solution_states.resize(state + 1);

    // The first three states (now, old, older) will point to the solutions in the libMesh system,
    // which is why we are using the "internal" calls to these vectors. _solution_states will then
    // be the forward facing access to these vectors
    _solution_states[0] = &solutionInternal();
    if (state > 0)
      _solution_states[1] = &solutionOldInternal();
    if (state > 1)
      _solution_states[2] = &solutionOlderInternal();

    // Create anything that is past older (state of 3+)
    for (unsigned int i = 3; i <= state; ++i)
      if (!_solution_states[i])
        _solution_states[i] = &addVector("solution_state_" + std::to_string(i), true, GHOSTED);
  }

  return *_solution_states[state];
}

void
SystemBase::applyScalingFactors(const std::vector<Real> & inverse_scaling_factors)
{
  for (MooseIndex(_vars) thread = 0; thread < _vars.size(); ++thread)
  {
    auto & field_variables = _vars[thread].fieldVariables();
    for (MooseIndex(field_variables) i = 0; i < field_variables.size(); ++i)
      field_variables[i]->scalingFactor(1. / inverse_scaling_factors[i] *
                                        field_variables[i]->scalingFactor());

    auto offset = field_variables.size();

    auto & scalar_variables = _vars[thread].scalars();
    for (MooseIndex(scalar_variables) i = 0; i < scalar_variables.size(); ++i)
      scalar_variables[i]->scalingFactor(1. / inverse_scaling_factors[offset + i] *
                                         scalar_variables[i]->scalingFactor());

    if (thread == 0 && _verbose)
    {
      _console << "Automatic scaling factors:\n";
      auto original_flags = _console.flags();
      auto original_precision = _console.precision();
      _console.unsetf(std::ios_base::floatfield);
      _console.precision(6);

      for (const auto & field_variable : field_variables)
        _console << "  " << field_variable->name() << ": " << field_variable->scalingFactor()
                 << "\n";
      for (const auto & scalar_variable : scalar_variables)
        _console << "  " << scalar_variable->name() << ": " << scalar_variable->scalingFactor()
                 << "\n";
      _console << "\n\n";

      // restore state
      _console.flags(original_flags);
      _console.precision(original_precision);
    }
  }
}

void
SystemBase::cacheVarIndicesByFace(const std::vector<VariableName> & vars)
{
  if (!_subproblem.haveFV())
    return;

  // prepare a vector of MooseVariables from names
  std::vector<MooseVariableBase *> moose_vars;
  for (auto & v : vars)
  {
    // first make sure this is not a scalar variable
    if (hasScalarVariable(v))
      mooseError("Variable ", v, " is a scalar variable");

    // now make sure this is a standard variable [not array/vector]
    if (getVariable(0, v).fieldType() != 0)
      mooseError("Variable ", v, " not a standard field variable [either VECTOR or ARRAY].");
    moose_vars.push_back(&getVariable(0, v));
  }

  // loop over all faces
  auto & faces = mesh().faceInfo();
  for (auto & p : faces)
  {
    // get elem & neighbor elements, and set subdomain ids
    const Elem & elem_elem = p.elem();
    const Elem * neighbor_elem = p.neighborPtr();
    SubdomainID elem_subdomain_id = elem_elem.subdomain_id();
    SubdomainID neighbor_subdomain_id = Elem::invalid_subdomain_id;
    if (neighbor_elem)
      neighbor_subdomain_id = neighbor_elem->subdomain_id();

    // TODO: what happens if elem and neighbor subdomain ids have different
    // coordinate transforms here?  Figure out how to handle this robustly.
    coordTransformFactor(_subproblem, elem_subdomain_id, p.faceCentroid(), p.faceCoord());

    // loop through vars
    for (unsigned int j = 0; j < moose_vars.size(); ++j)
    {
      // get the variable, its name, and its domain of definition
      auto var = moose_vars[j];
      auto var_name = var->name();
      std::set<SubdomainID> var_subdomains = var->blockIDs();

      // unfortunately, MOOSE is lazy and all subdomains has its own
      // ID. If ANY_BLOCK_ID is in var_subdomains, inject all subdomains explicitly
      if (var_subdomains.find(Moose::ANY_BLOCK_ID) != var_subdomains.end())
        var_subdomains = _mesh.meshSubdomains();

      // first stash away DoF information; this is more difficult than you would
      // think because var can be defined on the elem subdomain, the neighbor subdomain
      // or both subdomains
      // elem
      std::vector<dof_id_type> elem_dof_indices;
      if (var_subdomains.find(elem_subdomain_id) != var_subdomains.end())
        var->getDofIndices(&elem_elem, elem_dof_indices);
      else
        elem_dof_indices = {libMesh::DofObject::invalid_id};
      p.elemDofIndices(var_name) = elem_dof_indices;
      // neighbor
      std::vector<dof_id_type> neighbor_dof_indices;
      if (neighbor_elem && var_subdomains.find(neighbor_subdomain_id) != var_subdomains.end())
        var->getDofIndices(neighbor_elem, neighbor_dof_indices);
      else
        neighbor_dof_indices = {libMesh::DofObject::invalid_id};
      p.neighborDofIndices(var_name) = neighbor_dof_indices;

      /**
       * The following paragraph of code assigns the VarFaceNeighbors
       * 1. The face is an internal face of this variable if it is defined on
       *    the elem and neighbor subdomains
       * 2. The face is an invalid face of this variable if it is neither defined
       *    on the elem nor the neighbor subdomains
       * 3. If not 1. or 2. then this is a boundary for this variable and the else clause
       *    applies
       */
      bool var_defined_elem = var_subdomains.find(elem_subdomain_id) != var_subdomains.end();
      bool var_defined_neighbor =
          var_subdomains.find(neighbor_subdomain_id) != var_subdomains.end();
      if (var_defined_elem && var_defined_neighbor)
        p.faceType(var_name) = FaceInfo::VarFaceNeighbors::BOTH;
      else if (!var_defined_elem && !var_defined_neighbor)
        p.faceType(var_name) = FaceInfo::VarFaceNeighbors::NEITHER;
      else
      {
        // this is a boundary face for this variable, set elem or neighbor
        if (var_defined_elem)
          p.faceType(var_name) = FaceInfo::VarFaceNeighbors::ELEM;
        else if (var_defined_neighbor)
          p.faceType(var_name) = FaceInfo::VarFaceNeighbors::NEIGHBOR;
        else
          mooseError("Should never get here");
      }
    }
  }
}

template MooseVariableFE<Real> & SystemBase::getFieldVariable<Real>(THREAD_ID tid,
                                                                    const std::string & var_name);

template MooseVariableFE<RealVectorValue> &
SystemBase::getFieldVariable<RealVectorValue>(THREAD_ID tid, const std::string & var_name);

template MooseVariableFE<RealEigenVector> &
SystemBase::getFieldVariable<RealEigenVector>(THREAD_ID tid, const std::string & var_name);

template MooseVariableFE<Real> & SystemBase::getFieldVariable<Real>(THREAD_ID tid,
                                                                    unsigned int var_number);

template MooseVariableFE<RealVectorValue> &
SystemBase::getFieldVariable<RealVectorValue>(THREAD_ID tid, unsigned int var_number);

template MooseVariableFE<RealEigenVector> &
SystemBase::getFieldVariable<RealEigenVector>(THREAD_ID tid, unsigned int var_number);

template MooseVariableField<Real> &
SystemBase::getActualFieldVariable<Real>(THREAD_ID tid, const std::string & var_name);

template MooseVariableField<RealVectorValue> &
SystemBase::getActualFieldVariable<RealVectorValue>(THREAD_ID tid, const std::string & var_name);

template MooseVariableField<RealEigenVector> &
SystemBase::getActualFieldVariable<RealEigenVector>(THREAD_ID tid, const std::string & var_name);

template MooseVariableField<Real> &
SystemBase::getActualFieldVariable<Real>(THREAD_ID tid, unsigned int var_number);

template MooseVariableField<RealVectorValue> &
SystemBase::getActualFieldVariable<RealVectorValue>(THREAD_ID tid, unsigned int var_number);

template MooseVariableField<RealEigenVector> &
SystemBase::getActualFieldVariable<RealEigenVector>(THREAD_ID tid, unsigned int var_number);
