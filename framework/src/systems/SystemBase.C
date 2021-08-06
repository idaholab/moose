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
    _automatic_scaling(false),
    _verbose(false),
    _solution_states_initialized(false)
{
}

MooseVariableFieldBase &
SystemBase::getVariable(THREAD_ID tid, const std::string & var_name) const
{
  MooseVariableFieldBase * var =
      dynamic_cast<MooseVariableFieldBase *>(_vars[tid].getVariable(var_name));
  if (!var)
    mooseError("Variable '", var_name, "' does not exist in this system");
  return *var;
}

MooseVariableFieldBase &
SystemBase::getVariable(THREAD_ID tid, unsigned int var_number) const
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
MooseVariableFV<T> &
SystemBase::getFVVariable(THREAD_ID tid, const std::string & var_name)
{
  return *_vars[tid].getFVVariable<T>(var_name);
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
SystemBase::getScalarVariable(THREAD_ID tid, const std::string & var_name) const
{
  MooseVariableScalar * var = dynamic_cast<MooseVariableScalar *>(_vars[tid].getVariable(var_name));
  if (!var)
    mooseError("Scalar variable '" + var_name + "' does not exist in this system");
  return *var;
}

MooseVariableScalar &
SystemBase::getScalarVariable(THREAD_ID tid, unsigned int var_number) const
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

    auto problem = dynamic_cast<FEProblemBase *>(&_subproblem);
    if (!problem)
      mooseError("System needs to be registered in FEProblemBase for using zeroVariables.");

    AllLocalDofIndicesThread aldit(*problem, vars_to_be_zeroed, true);
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
  const std::vector<MooseVariableFieldBase *> & vars = _vars[0].fieldVariables();
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
    const std::set<MooseVariableFieldBase *> & active_elemental_moose_variables =
        _subproblem.getActiveElementalMooseVariables(tid);
    const std::vector<MooseVariableFieldBase *> & vars = _vars[tid].fieldVariables();
    for (const auto & var : vars)
      var->clearDofIndices();

#ifndef MOOSE_GLOBAL_AD_INDEXING
    // When we have a displaced problem and we have AD objects it's possible that you can have
    // something like the following: A displaced displacement kernel uses a material property
    // computed with an undisplaced material. That material property is a function of the
    // temperature. However, the temperature doesn't have any displaced kernels acting on it nor has
    // any displaced objects explicitly coupling it. In this case the displaced temperature would
    // not register as an active_elemental_moose_variable, and if we only prepare
    // active_elemental_moose_variables then in our ADKernel (when not using global AD indexing) we
    // will either have to skip over variables who have no dof indices or we will attempt to index
    // out of bounds into _local_ke. So we need to make sure here that we prepare all variables that
    // are active *either* in the undisplaced or displaced system
    if (_subproblem.haveADObjects() && _subproblem.haveDisplaced())
    {
      // If active_elemental_moose_variables contains both copies of an undisplaced and displaced
      // variable, use this container to make sure we don't prepare said variable twice
      std::set<unsigned int> vars_initd;
      for (auto * const var : active_elemental_moose_variables)
      {
        // eliminate variables that are not of like nl/aux system
        if (var->kind() != _var_kind)
          continue;

        const unsigned int var_num = var->number();
        if (vars_initd.find(var_num) == vars_initd.end())
        {
          if (&var->sys() == this)
            var->prepare();
          else
            this->getVariable(tid, var_num).prepare();

          vars_initd.insert(var_num);
        }
      }
    }
    else
#endif
      for (const auto & var : active_elemental_moose_variables)
        if (&(var->sys()) == this)
          var->prepare();
  }
  else
  {
    const std::vector<MooseVariableFieldBase *> & vars = _vars[tid].fieldVariables();
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
    const std::set<MooseVariableFieldBase *> & active_elemental_moose_variables =
        _subproblem.getActiveElementalMooseVariables(tid);

    std::vector<MooseVariableFieldBase *> newly_prepared_vars;

    const std::vector<MooseVariableFieldBase *> & vars = _vars[tid].fieldVariables();
    for (const auto & var : vars)
    {
      mooseAssert(&var->sys() == this,
                  "I will cry if we store variables in our warehouse that don't belong to us");

      // If it wasn't in the active list, we need to prepare it. This has the potential to duplicate
      // prepare if we have these conditions:
      //
      // 1. We have a displaced problem
      // 2. We are using AD
      // 3. We are not using global AD indexing
      //
      // But I think I would rather risk duplicate prepare than introduce an additional member set
      // variable for tracking prepared variables. Set insertion is slow and some simulations have a
      // ton of variables
      if (!active_elemental_moose_variables.count(var))
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
  const std::vector<MooseVariableFieldBase *> & vars = _vars[tid].fieldVariables();
  for (const auto & var : vars)
    var->prepareNeighbor();
}

void
SystemBase::prepareLowerD(THREAD_ID tid)
{
  const std::vector<MooseVariableFieldBase *> & vars = _vars[tid].fieldVariables();
  for (const auto & var : vars)
    var->prepareLowerD();
}

void
SystemBase::reinitElem(const Elem * /*elem*/, THREAD_ID tid)
{

  if (_subproblem.hasActiveElementalMooseVariables(tid))
  {
    const std::set<MooseVariableFieldBase *> & active_elemental_moose_variables =
        _subproblem.getActiveElementalMooseVariables(tid);
    for (const auto & var : active_elemental_moose_variables)
      if (&(var->sys()) == this)
        var->computeElemValues();
  }
  else
  {
    const std::vector<MooseVariableFieldBase *> & vars = _vars[tid].fieldVariables();
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
  const std::vector<MooseVariableFieldBase *> & vars = _vars[tid].fieldVariables();
  for (const auto & var : vars)
    var->computeElemValuesFace();
}

void
SystemBase::reinitNeighborFace(const Elem * /*elem*/,
                               unsigned int /*side*/,
                               BoundaryID /*bnd_id*/,
                               THREAD_ID tid)
{
  const std::vector<MooseVariableFieldBase *> & vars = _vars[tid].fieldVariables();
  for (const auto & var : vars)
    var->computeNeighborValuesFace();
}

void
SystemBase::reinitNeighbor(const Elem * /*elem*/, THREAD_ID tid)
{
  const std::vector<MooseVariableFieldBase *> & vars = _vars[tid].fieldVariables();
  for (const auto & var : vars)
    var->computeNeighborValues();
}

void
SystemBase::reinitLowerD(THREAD_ID tid)
{
  const std::vector<MooseVariableFieldBase *> & vars = _vars[tid].fieldVariables();
  for (const auto & var : vars)
    var->computeLowerDValues();
}

void
SystemBase::reinitNode(const Node * /*node*/, THREAD_ID tid)
{
  const std::vector<MooseVariableFieldBase *> & vars = _vars[tid].fieldVariables();
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
  const std::vector<MooseVariableFieldBase *> & vars = _vars[tid].fieldVariables();
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
  const std::vector<MooseVariableFieldBase *> & vars = _vars[tid].fieldVariables();
  for (const auto & var : vars)
  {
    var->reinitNodes(nodes);
    var->computeNodalValues();
  }
}

void
SystemBase::reinitNodesNeighbor(const std::vector<dof_id_type> & nodes, THREAD_ID tid)
{
  const std::vector<MooseVariableFieldBase *> & vars = _vars[tid].fieldVariables();
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

SparseMatrix<Number> &
SystemBase::addMatrix(TagID tag)
{
  if (!_subproblem.matrixTagExists(tag))
    mooseError("Cannot add tagged matrix with TagID ",
               tag,
               " in system '",
               name(),
               "' because the tag does not exist in the problem");

  if (hasMatrix(tag))
    return getMatrix(tag);

  const auto matrix_name = _subproblem.matrixTagName(tag);
  SparseMatrix<Number> & mat = system().add_matrix(matrix_name);
  associateMatrixToTag(mat, tag);

  return mat;
}

void
SystemBase::removeMatrix(TagID tag_id)
{
  if (!_subproblem.matrixTagExists(tag_id))
    mooseError("Cannot remove the matrix with TagID ",
               tag_id,
               "\nin system '",
               name(),
               "', because that tag does not exist in the problem");

  if (hasMatrix(tag_id))
  {
    const auto matrix_name = _subproblem.matrixTagName(tag_id);
    system().remove_matrix(matrix_name);
    _tagged_matrices[tag_id] = nullptr;
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
    mooseError("Cannot add tagged vector with TagID ",
               tag,
               " in system '",
               name(),
               "' because the tag does not exist in the problem");

  if (hasVector(tag))
  {
    auto & vec = getVector(tag);

    if (type != ParallelType::AUTOMATIC && vec.type() != type)
      mooseError("Cannot add tagged vector '",
                 _subproblem.vectorTagName(tag),
                 "', in system '",
                 name(),
                 "' because a vector with the same name was found with a different parallel type");

    return vec;
  }

  const auto vector_name = _subproblem.vectorTagName(tag);
  NumericVector<Number> & vec = system().add_vector(vector_name, project, type);
  associateVectorToTag(vec, tag);

  return vec;
}

void
SystemBase::closeTaggedVector(const TagID tag)
{
  if (!_subproblem.vectorTagExists(tag))
    mooseError("Cannot close vector with TagID ",
               tag,
               " in system '",
               name(),
               "' because that tag does not exist in the problem");
  else if (!hasVector(tag))
    mooseError("Cannot close vector tag with name '",
               _subproblem.vectorTagName(tag),
               "' in system '",
               name(),
               "' because there is no vector associated with that tag");

  getVector(tag).close();
}

void
SystemBase::closeTaggedVectors(const std::set<TagID> & tags)
{
  for (const auto tag : tags)
    closeTaggedVector(tag);
}

void
SystemBase::zeroTaggedVector(const TagID tag)
{
  if (!_subproblem.vectorTagExists(tag))
    mooseError("Cannot zero vector with TagID ",
               tag,
               " in system '",
               name(),
               "' because that tag does not exist in the problem");
  else if (!hasVector(tag))
    mooseError("Cannot zero vector tag with name '",
               _subproblem.vectorTagName(tag),
               "' in system '",
               name(),
               "' because there is no vector associated with that tag");

  getVector(tag).zero();
}

void
SystemBase::zeroTaggedVectors(const std::set<TagID> & tags)
{
  for (const auto tag : tags)
    zeroTaggedVector(tag);
}

void
SystemBase::removeVector(TagID tag_id)
{
  if (!_subproblem.vectorTagExists(tag_id))
    mooseError("Cannot remove the vector with TagID ",
               tag_id,
               "\nin system '",
               name(),
               "', because that tag does not exist in the problem");

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

    // Set as array variable
    if (parameters.isParamSetByUser("array") && !parameters.get<bool>("array"))
      mooseError("Variable '",
                 name,
                 "' is an array variable ('components' > 1) but 'array' is set to false.");
    parameters.set<bool>("array") = true;
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

    if (auto fe_var = dynamic_cast<MooseVariableFieldBase *>(var.get()))
    {
      auto required_size = var_num + components;
      if (required_size > _numbered_vars[tid].size())
        _numbered_vars[tid].resize(required_size);
      for (MooseIndex(components) component = 0; component < components; ++component)
        _numbered_vars[tid][var_num + component] = fe_var;

      if (auto * const functor = dynamic_cast<Moose::FunctorBase<ADReal> *>(fe_var))
        _subproblem.addFunctor(name, *functor, tid);
      else if (auto * const functor = dynamic_cast<Moose::FunctorBase<ADRealVectorValue> *>(fe_var))
        _subproblem.addFunctor(name, *functor, tid);
      else if (auto * const functor = dynamic_cast<Moose::FunctorBase<RealEigenVector> *>(fe_var))
        _subproblem.addFunctor(name, *functor, tid);
      else
        mooseError("This should be a functor");
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

/**
 * Get a raw NumericVector with the given name.
 */
NumericVector<Number> &
SystemBase::getVector(const std::string & name)
{
  return system().get_vector(name);
}

const NumericVector<Number> &
SystemBase::getVector(const std::string & name) const
{
  return system().get_vector(name);
}

NumericVector<Number> &
SystemBase::getVector(TagID tag)
{
  if (!hasVector(tag))
  {
    if (!_subproblem.vectorTagExists(tag))
      mooseError("Cannot retreive vector with tag ", tag, " because that tag does not exist");
    else
      mooseError("Cannot retreive vector with tag ",
                 tag,
                 " in system '",
                 name(),
                 "'\nbecause a vector has not been associated with that tag.");
  }

  return *_tagged_vectors[tag];
}

const NumericVector<Number> &
SystemBase::getVector(TagID tag) const
{
  if (!hasVector(tag))
  {
    if (!_subproblem.vectorTagExists(tag))
      mooseError("Cannot retreive vector with tag ", tag, " because that tag does not exist");
    else
      mooseError("Cannot retreive vector with tag ",
                 tag,
                 " in system '",
                 name(),
                 "'\nbecause a vector has not been associated with that tag.");
  }

  return *_tagged_vectors[tag];
}

void
SystemBase::associateVectorToTag(NumericVector<Number> & vec, TagID tag)
{
  if (!_subproblem.vectorTagExists(tag))
    mooseError("Cannot associate vector to tag ", tag, " because that tag does not exist");

  if (_tagged_vectors.size() < tag + 1)
    _tagged_vectors.resize(tag + 1);

  _tagged_vectors[tag] = &vec;
}

void
SystemBase::disassociateVectorFromTag(NumericVector<Number> & vec, TagID tag)
{
  if (!_subproblem.vectorTagExists(tag))
    mooseError("Cannot disassociate vector from tag ", tag, " because that tag does not exist");
  if (hasVector(tag) && &getVector(tag) != &vec)
    mooseError("You can not disassociate a vector from a tag which it was not associated to");

  disassociateVectorFromTag(tag);
}

void
SystemBase::disassociateVectorFromTag(TagID tag)
{
  if (!_subproblem.vectorTagExists(tag))
    mooseError("Cannot disassociate vector from tag ", tag, " because that tag does not exist");

  if (_tagged_vectors.size() < tag + 1)
    _tagged_vectors.resize(tag + 1);
  _tagged_vectors[tag] = nullptr;
}

void
SystemBase::disassociateDefaultVectorTags()
{
  const auto tags = defaultVectorTags();
  for (const auto tag : tags)
    if (_subproblem.vectorTagExists(tag))
      disassociateVectorFromTag(tag);
}

SparseMatrix<Number> &
SystemBase::getMatrix(TagID tag)
{
  if (!hasMatrix(tag))
  {
    if (!_subproblem.matrixTagExists(tag))
      mooseError("Cannot retreive matrix with tag ", tag, " because that tag does not exist");
    else
      mooseError("Cannot retreive matrix with tag ",
                 tag,
                 " in system '",
                 name(),
                 "'\nbecause a matrix has not been associated with that tag.");
  }

  return *_tagged_matrices[tag];
}

const SparseMatrix<Number> &
SystemBase::getMatrix(TagID tag) const
{
  if (!hasMatrix(tag))
  {
    if (!_subproblem.matrixTagExists(tag))
      mooseError("Cannot retreive matrix with tag ", tag, " because that tag does not exist");
    else
      mooseError("Cannot retreive matrix with tag ",
                 tag,
                 " in system '",
                 name(),
                 "'\nbecause a matrix has not been associated with that tag.");
  }

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
SystemBase::flushTaggedMatrices(const std::set<TagID> & tags)
{
  for (auto tag : tags)
    if (hasMatrix(tag))
      getMatrix(tag).flush();
}

void
SystemBase::associateMatrixToTag(SparseMatrix<Number> & matrix, TagID tag)
{
  if (!_subproblem.matrixTagExists(tag))
    mooseError("Cannot associate matrix to tag ", tag, " because that tag does not exist");

  if (_tagged_matrices.size() < tag + 1)
    _tagged_matrices.resize(tag + 1);

  _tagged_matrices[tag] = &matrix;
}

void
SystemBase::disassociateMatrixFromTag(SparseMatrix<Number> & matrix, TagID tag)
{
  if (!_subproblem.matrixTagExists(tag))
    mooseError("Cannot disassociate matrix from tag ", tag, " because that tag does not exist");
  if (hasMatrix(tag) && &getMatrix(tag) != &matrix)
    mooseError("You can not disassociate a matrix from a tag which it was not associated to");

  disassociateMatrixFromTag(tag);
}

void
SystemBase::disassociateMatrixFromTag(TagID tag)
{
  if (!_subproblem.matrixTagExists(tag))
    mooseError("Cannot disassociate matrix from tag ", tag, " because that tag does not exist");

  if (_tagged_matrices.size() < tag + 1)
    _tagged_matrices.resize(tag + 1);
  _tagged_matrices[tag] = nullptr;
}

void
SystemBase::disassociateDefaultMatrixTags()
{
  const auto tags = defaultMatrixTags();
  for (const auto tag : tags)
    if (_subproblem.matrixTagExists(tag))
      disassociateMatrixFromTag(tag);
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
      const auto & var = getVariable(0, vci._dest_name);
      if (var.count() > 1) // array variable
      {
        const auto & array_var = getFieldVariable<RealEigenVector>(0, vci._dest_name);
        for (MooseIndex(var.count()) i = 0; i < var.count(); ++i)
        {
          const auto exodus_var = _subproblem.arrayVariableComponent(vci._source_name, i);
          const auto system_var = array_var.componentName(i);
          if (var.isNodal())
            io.copy_nodal_solution(system(), exodus_var, system_var, timestep);
          else
            io.copy_elemental_solution(system(), exodus_var, system_var, timestep);
        }
      }
      else
      {
        if (var.isNodal())
          io.copy_nodal_solution(system(), vci._dest_name, vci._source_name, timestep);
        else
          io.copy_elemental_solution(system(), vci._dest_name, vci._source_name, timestep);
      }
    }
    else if (hasScalarVariable(vci._dest_name))
      io.copy_scalar_solution(system(), {vci._dest_name}, {vci._source_name}, timestep);
    else
      mooseError("Unrecognized variable ", vci._dest_name, " in variables to copy.");
  }

  if (did_copy)
    solution().close();
}

void
SystemBase::update(const bool update_libmesh_system)
{
  if (update_libmesh_system)
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
  if (!hasSolutionState(1))
    mooseError("Cannot restore solutions without old solution");

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

NumericVector<Number> *
SystemBase::solutionPreviousNewton()
{
  if (hasVector(Moose::PREVIOUS_NL_SOLUTION_TAG))
    return &getVector(Moose::PREVIOUS_NL_SOLUTION_TAG);
  else
    return nullptr;
}

const NumericVector<Number> *
SystemBase::solutionPreviousNewton() const
{
  if (hasVector(Moose::PREVIOUS_NL_SOLUTION_TAG))
    return &getVector(Moose::PREVIOUS_NL_SOLUTION_TAG);
  else
    return nullptr;
}

void
SystemBase::initSolutionState()
{
  // Default is the current solution
  unsigned int state = 0;

  // Add additional states as required by the variable states requested
  for (const auto & var : getVariables(/* tid = */ 0))
    state = std::max(state, var->oldestSolutionStateRequested());
  for (const auto & var : getScalarVariables(/* tid = */ 0))
    state = std::max(state, var->oldestSolutionStateRequested());

  needSolutionState(state);

  _solution_states_initialized = true;
}

TagName
SystemBase::oldSolutionStateVectorName(const unsigned int state) const
{
  mooseAssert(state != 0, "Not an old state");
  if (state == 1)
    return Moose::OLD_SOLUTION_TAG;
  else if (state == 2)
    return Moose::OLDER_SOLUTION_TAG;
  else
    return "solution_state_" + std::to_string(state);
}

const NumericVector<Number> &
SystemBase::solutionState(const unsigned int state) const
{
  if (!hasSolutionState(state))
    mooseError("Solution state ",
               state,
               " was requested in ",
               name(),
               " but only up to state ",
               _solution_states.size() - 1,
               " is available.");

  if (state == 0)
    mooseAssert(_solution_states[0] == &solutionInternal(), "Inconsistent current solution");
  else
    mooseAssert(_solution_states[state] == &getVector(oldSolutionStateVectorName(state)),
                "Inconsistent solution state");

  return *_solution_states[state];
}

NumericVector<Number> &
SystemBase::solutionState(const unsigned int state)
{
  if (!hasSolutionState(state))
    needSolutionState(state);
  return *_solution_states[state];
}

void
SystemBase::needSolutionState(const unsigned int state)
{
  if (hasSolutionState(state))
    return;

  _solution_states.resize(state + 1);

  // The 0-th (current) solution state is owned by libMesh
  if (!_solution_states[0])
    _solution_states[0] = &solutionInternal();
  else
    mooseAssert(_solution_states[0] == &solutionInternal(), "Inconsistent current solution");

  // We will manually add all states past current
  for (unsigned int i = 1; i <= state; ++i)
    if (!_solution_states[i])
    {
      auto tag =
          _subproblem.addVectorTag(oldSolutionStateVectorName(i), Moose::VECTOR_TAG_SOLUTION);
      _solution_states[i] = &addVector(tag, true, GHOSTED);
    }
    else
      mooseAssert(_solution_states[i] == &getVector(oldSolutionStateVectorName(i)),
                  "Inconsistent solution state");
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
      _console << "\n" << std::endl;

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
  std::vector<const MooseVariableBase *> moose_vars;
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

  _mesh.cacheVarIndicesByFace(moose_vars);
  _mesh.computeFaceInfoFaceCoords();
}

#ifdef MOOSE_GLOBAL_AD_INDEXING
void
SystemBase::addScalingVector()
{
  addVector("scaling_factors", /*project=*/false, libMesh::ParallelType::GHOSTED);
  _subproblem.hasScalingVector();
}
#endif

bool
SystemBase::computingScalingJacobian() const
{
  return _subproblem.computingScalingJacobian();
}

void
SystemBase::initialSetup()
{
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
    _vars[tid].initialSetup();
}

void
SystemBase::timestepSetup()
{
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
    _vars[tid].timestepSetup();
}

void
SystemBase::subdomainSetup()
{
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
    _vars[tid].subdomainSetup();
}

void
SystemBase::residualSetup()
{
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
    _vars[tid].residualSetup();
}

void
SystemBase::jacobianSetup()
{
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
    _vars[tid].jacobianSetup();
}

void
SystemBase::clearAllDofIndices()
{
  for (auto & var_warehouse : _vars)
    var_warehouse.clearAllDofIndices();
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

template MooseVariableFV<Real> & SystemBase::getFVVariable<Real>(THREAD_ID tid,
                                                                 const std::string & var_name);
