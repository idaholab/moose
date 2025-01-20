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
#include "FEProblemBase.h"
#include "TimeIntegrator.h"

#include "libmesh/dof_map.h"
#include "libmesh/string_to_enum.h"
#include "libmesh/fe_interface.h"

using namespace libMesh;

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

SystemBase::SystemBase(SubProblem & subproblem,
                       FEProblemBase & fe_problem,
                       const std::string & name,
                       Moose::VarKindType var_kind)
  : libMesh::ParallelObject(subproblem),
    ConsoleStreamInterface(subproblem.getMooseApp()),
    _subproblem(subproblem),
    _fe_problem(fe_problem),
    _app(subproblem.getMooseApp()),
    _factory(_app.getFactory()),
    _mesh(subproblem.mesh()),
    _name(name),
    _vars(libMesh::n_threads()),
    _var_map(),
    _max_var_number(0),
    _u_dot(nullptr),
    _u_dotdot(nullptr),
    _u_dot_old(nullptr),
    _u_dotdot_old(nullptr),
    _saved_old(nullptr),
    _saved_older(nullptr),
    _saved_dot_old(nullptr),
    _saved_dotdot_old(nullptr),
    _var_kind(var_kind),
    _max_var_n_dofs_per_elem(0),
    _max_var_n_dofs_per_node(0),
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
SystemBase::setVariableGlobalDoFs(const std::string & var_name)
{
  AllLocalDofIndicesThread aldit(_subproblem, {var_name});
  ConstElemRange & elem_range = *_mesh.getActiveLocalElementRange();
  Threads::parallel_reduce(elem_range, aldit);

  // Gather the dof indices across procs to get all the dof indices for var_name
  aldit.dofIndicesSetUnion();

  const auto & all_dof_indices = aldit.getDofIndices();
  _var_all_dof_indices.assign(all_dof_indices.begin(), all_dof_indices.end());
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
        _subproblem.assembly(tid, number()).prepareVariable(var_ptr);
        if (_subproblem.checkNonlocalCouplingRequirement())
          _subproblem.assembly(tid, number()).prepareVariableNonlocal(var_ptr);
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
SystemBase::reinitElemFace(const Elem * /*elem*/, unsigned int /*side*/, THREAD_ID tid)
{
  const std::vector<MooseVariableFieldBase *> & vars = _vars[tid].fieldVariables();
  for (const auto & var : vars)
    var->computeElemValuesFace();
}

void
SystemBase::reinitNeighborFace(const Elem * /*elem*/, unsigned int /*side*/, THREAD_ID tid)
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
  const auto states =
      _solution_states[static_cast<unsigned short>(Moose::SolutionIterationType::Time)].size();
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
  const auto states =
      _solution_states[static_cast<unsigned short>(Moose::SolutionIterationType::Time)].size();
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
  if (!_subproblem.vectorTagNotZeroed(tag))
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

  const auto fe_type =
      FEType(Utility::string_to_enum<Order>(parameters.get<MooseEnum>("order")),
             Utility::string_to_enum<FEFamily>(parameters.get<MooseEnum>("family")));
  const auto fe_field_type = FEInterface::field_type(fe_type);

  unsigned int var_num;

  if (var_type == "ArrayMooseVariable")
  {
    if (fe_field_type == TYPE_VECTOR)
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
  _du_dot_du.resize(var_num + 1);
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
  unsigned int n = nFieldVariables();
  n += _vars[0].scalars().size();

  return n;
}

unsigned int
SystemBase::nFieldVariables() const
{
  unsigned int n = 0;
  for (auto & var : _vars[0].fieldVariables())
    n += var->count();

  return n;
}

unsigned int
SystemBase::nFVVariables() const
{
  unsigned int n = 0;
  for (auto & var : _vars[0].fieldVariables())
    if (var->isFV())
      n += var->count();

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
      if (var.isArray())
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
  copyOldSolutions();
  copyPreviousNonlinearSolutions();
}

/**
 * Shifts the solutions backwards in nonlinear iteration history
 */
void
SystemBase::copyPreviousNonlinearSolutions()
{
  // 1 is for nonlinear, 0 is for time, we do this for nonlinear only here
  const auto states = _solution_states[1].size();
  if (states > 1)
    for (unsigned int i = states - 1; i > 0; --i)
      solutionState(i, Moose::SolutionIterationType::Nonlinear) =
          solutionState(i - 1, Moose::SolutionIterationType::Nonlinear);

  if (solutionPreviousNewton())
    *solutionPreviousNewton() = *currentSolution();
}

/**
 * Shifts the solutions backwards in time
 */
void
SystemBase::copyOldSolutions()
{
  // Copying the solutions backward so the current solution will become the old, and the old will
  // become older. 0 index is for time, 1 would be nonlinear iteration.
  const auto states = _solution_states[0].size();
  if (states > 1)
    for (unsigned int i = states - 1; i > 0; --i)
      solutionState(i) = solutionState(i - 1);

  if (solutionUDotOld())
    *solutionUDotOld() = *solutionUDot();
  if (solutionUDotDotOld())
    *solutionUDotDotOld() = *solutionUDotDot();
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

  needSolutionState(state, Moose::SolutionIterationType::Time);

  _solution_states_initialized = true;
}

TagName
SystemBase::oldSolutionStateVectorName(const unsigned int state,
                                       const Moose::SolutionIterationType iteration_type) const
{
  mooseAssert(state != 0, "Not an old state");

  if (iteration_type == Moose::SolutionIterationType::Time)
  {
    if (state == 1)
      return Moose::OLD_SOLUTION_TAG;
    else if (state == 2)
      return Moose::OLDER_SOLUTION_TAG;
  }
  else if (iteration_type == Moose::SolutionIterationType::Nonlinear && state == 1)
    return Moose::PREVIOUS_NL_SOLUTION_TAG;

  return "solution_state_" + std::to_string(state) + "_" + Moose::stringify(iteration_type);
}

const NumericVector<Number> &
SystemBase::solutionState(const unsigned int state,
                          const Moose::SolutionIterationType iteration_type) const
{
  if (!hasSolutionState(state, iteration_type))
    mooseError("For iteration type '",
               Moose::stringify(iteration_type),
               "': solution state ",
               state,
               " was requested in ",
               name(),
               " but only up to state ",
               (_solution_states[static_cast<unsigned short>(iteration_type)].size() == 0)
                   ? 0
                   : _solution_states[static_cast<unsigned short>(iteration_type)].size() - 1,
               " is available.");

  const auto & solution_states = _solution_states[static_cast<unsigned short>(iteration_type)];

  if (state == 0)
    mooseAssert(solution_states[0] == &solutionInternal(), "Inconsistent current solution");
  else
    mooseAssert(solution_states[state] ==
                    &getVector(oldSolutionStateVectorName(state, iteration_type)),
                "Inconsistent solution state");

  return *solution_states[state];
}

NumericVector<Number> &
SystemBase::solutionState(const unsigned int state,
                          const Moose::SolutionIterationType iteration_type)
{
  if (!hasSolutionState(state, iteration_type))
    needSolutionState(state, iteration_type);
  return *_solution_states[static_cast<unsigned short>(iteration_type)][state];
}

void
SystemBase::needSolutionState(const unsigned int state,
                              const Moose::SolutionIterationType iteration_type)
{
  libmesh_parallel_only(this->comm());
  mooseAssert(!Threads::in_threads,
              "This routine is not thread-safe. Request the solution state before using it in "
              "a threaded region.");

  if (hasSolutionState(state, iteration_type))
    return;

  auto & solution_states = _solution_states[static_cast<unsigned short>(iteration_type)];
  solution_states.resize(state + 1);

  // The 0-th (current) solution state is owned by libMesh
  if (!solution_states[0])
    solution_states[0] = &solutionInternal();
  else
    mooseAssert(solution_states[0] == &solutionInternal(), "Inconsistent current solution");

  // We will manually add all states past current
  for (unsigned int i = 1; i <= state; ++i)
    if (!solution_states[i])
    {
      auto tag = _subproblem.addVectorTag(oldSolutionStateVectorName(i, iteration_type),
                                          Moose::VECTOR_TAG_SOLUTION);
      solution_states[i] = &addVector(tag, true, GHOSTED);
    }
    else
      mooseAssert(solution_states[i] == &getVector(oldSolutionStateVectorName(i, iteration_type)),
                  "Inconsistent solution state");
}

void
SystemBase::applyScalingFactors(const std::vector<Real> & inverse_scaling_factors)
{
  for (MooseIndex(_vars) thread = 0; thread < _vars.size(); ++thread)
  {
    auto & field_variables = _vars[thread].fieldVariables();
    for (MooseIndex(field_variables) i = 0, p = 0; i < field_variables.size(); ++i)
    {
      auto factors = field_variables[i]->arrayScalingFactor();
      for (unsigned int j = 0; j < field_variables[i]->count(); ++j, ++p)
        factors[j] /= inverse_scaling_factors[p];

      field_variables[i]->scalingFactor(factors);
    }

    auto offset = field_variables.size();

    auto & scalar_variables = _vars[thread].scalars();
    for (MooseIndex(scalar_variables) i = 0; i < scalar_variables.size(); ++i)
      scalar_variables[i]->scalingFactor(
          {1. / inverse_scaling_factors[offset + i] * scalar_variables[i]->scalingFactor()});

    if (thread == 0 && _verbose)
    {
      _console << "Automatic scaling factors:\n";
      auto original_flags = _console.flags();
      auto original_precision = _console.precision();
      _console.unsetf(std::ios_base::floatfield);
      _console.precision(6);

      for (const auto & field_variable : field_variables)
      {
        const auto & factors = field_variable->arrayScalingFactor();
        _console << "  " << field_variable->name() << ":";
        for (const auto i : make_range(field_variable->count()))
          _console << " " << factors[i];
        _console << "\n";
      }
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
SystemBase::addScalingVector()
{
  addVector("scaling_factors", /*project=*/false, libMesh::ParallelType::GHOSTED);
  _subproblem.hasScalingVector(number());
}

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

  // If we need raw gradients, we initialize them here.
  bool gradient_storage_initialized = false;
  for (const auto & field_var : _vars[0].fieldVariables())
    if (!gradient_storage_initialized && field_var->needsGradientVectorStorage())
    {
      _raw_grad_container.clear();
      for (const auto i : make_range(this->_mesh.dimension()))
      {
        libmesh_ignore(i);
        _raw_grad_container.push_back(currentSolution()->zero_clone());
      }
    }
}

void
SystemBase::timestepSetup()
{
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
    _vars[tid].timestepSetup();
}

void
SystemBase::customSetup(const ExecFlagType & exec_type)
{
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
    _vars[tid].customSetup(exec_type);
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

void
SystemBase::setActiveVariableCoupleableVectorTags(const std::set<TagID> & vtags, THREAD_ID tid)
{
  _vars[tid].setActiveVariableCoupleableVectorTags(vtags);
}

void
SystemBase::setActiveScalarVariableCoupleableVectorTags(const std::set<TagID> & vtags,
                                                        THREAD_ID tid)
{
  _vars[tid].setActiveScalarVariableCoupleableVectorTags(vtags);
}

void
SystemBase::addDotVectors()
{
  if (_fe_problem.uDotRequested())
    _u_dot = &addVector("u_dot", true, GHOSTED);
  if (_fe_problem.uDotOldRequested())
    _u_dot_old = &addVector("u_dot_old", true, GHOSTED);
  if (_fe_problem.uDotDotRequested())
    _u_dotdot = &addVector("u_dotdot", true, GHOSTED);
  if (_fe_problem.uDotDotOldRequested())
    _u_dotdot_old = &addVector("u_dotdot_old", true, GHOSTED);
}

NumericVector<Number> &
SystemBase::serializedSolution()
{
  if (!_serialized_solution.get())
  {
    _serialized_solution = NumericVector<Number>::build(_communicator);
    _serialized_solution->init(system().n_dofs(), false, SERIAL);
  }

  return *_serialized_solution;
}

void
SystemBase::addTimeIntegrator(const std::string & type,
                              const std::string & name,
                              InputParameters & parameters)
{
  parameters.set<SystemBase *>("_sys") = this;
  _time_integrators.push_back(_factory.create<TimeIntegrator>(type, name, parameters));
}

void
SystemBase::copyTimeIntegrators(const SystemBase & other_sys)
{
  _time_integrators = other_sys._time_integrators;
}

const TimeIntegrator *
SystemBase::queryTimeIntegrator(const unsigned int var_num) const
{
  for (auto & ti : _time_integrators)
    if (ti->integratesVar(var_num))
      return ti.get();

  return nullptr;
}

const TimeIntegrator &
SystemBase::getTimeIntegrator(const unsigned int var_num) const
{
  const auto * const ti = queryTimeIntegrator(var_num);

  if (ti)
    return *ti;
  else
    mooseError("No time integrator found that integrates variable number ",
               std::to_string(var_num));
}

const std::vector<std::shared_ptr<TimeIntegrator>> &
SystemBase::getTimeIntegrators()
{
  return _time_integrators;
}

const Number &
SystemBase::duDotDu(const unsigned int var_num) const
{
  return _du_dot_du[var_num];
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
