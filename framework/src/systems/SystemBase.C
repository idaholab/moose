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
    _dummy_vec(nullptr),
    _saved_old(nullptr),
    _saved_older(nullptr),
    _var_kind(var_kind)
{
}

MooseVariableFEBase &
SystemBase::getVariable(THREAD_ID tid, const std::string & var_name)
{
  MooseVariableFEBase * var = dynamic_cast<MooseVariableFEBase *>(_vars[tid].getVariable(var_name));
  if (!var)
    mooseError("Variable '" + var_name + "' does not exist in this system");
  return *var;
}

MooseVariableFEBase &
SystemBase::getVariable(THREAD_ID tid, unsigned int var_number)
{
  MooseVariableFEBase * var =
      dynamic_cast<MooseVariableFEBase *>(_vars[tid].getVariable(var_number));
  if (!var)
    mooseError("variable #" + Moose::stringify(var_number) + " does not exist in this system");
  return *var;
}

template <typename T>
MooseVariableFE<T> &
SystemBase::getFieldVariable(THREAD_ID tid, const std::string & var_name)
{
  return *_vars[tid].getFieldVariable<T>(var_name);
}

template <typename T>
MooseVariableFE<T> &
SystemBase::getFieldVariable(THREAD_ID tid, unsigned int var_number)
{
  return *_vars[tid].getFieldVariable<T>(var_number);
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

    const std::set<dof_id_type> & dof_indices_to_zero = aldit._all_dof_indices;

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
SystemBase::reinitNode(const Node * /*node*/, THREAD_ID tid)
{
  const std::vector<MooseVariableFEBase *> & vars = _vars[tid].fieldVariables();
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
  const std::vector<MooseVariableFEBase *> & vars = _vars[tid].fieldVariables();
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
    _saved_old = nullptr;
  }
  if (_saved_older)
  {
    solutionOlder() = *_saved_older;
    removeVector("save_solution_older");
    _saved_older = nullptr;
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
    MooseVariableBase * var;
    if (type == FEType(0, MONOMIAL))
      var = new MooseVariableConstMonomial(
          var_num, type, *this, _subproblem.assembly(tid), _var_kind);
    else if (type == FEType(FIRST, NEDELEC_ONE) || type.family == LAGRANGE_VEC)
      var = new VectorMooseVariable(var_num, type, *this, _subproblem.assembly(tid), _var_kind);
    else
      var = new MooseVariable(var_num, type, *this, _subproblem.assembly(tid), _var_kind);

    var->scalingFactor(scale_factor);
    _vars[tid].add(var_name, var);
  }
  if (active_subdomains == nullptr)
    _var_map[var_num] = std::set<SubdomainID>();
  else
    for (const auto subdomain_id : *active_subdomains)
      _var_map[var_num].insert(subdomain_id);
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
  if (active_subdomains == nullptr)
    _var_map[var_num] = std::set<SubdomainID>();
  else
    for (const auto subdomain_id : *active_subdomains)
      _var_map[var_num].insert(subdomain_id);
}

bool
SystemBase::hasVariable(const std::string & var_name) const
{
  if (system().has_variable(var_name))
    return system().variable_type(var_name).family != SCALAR;
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
  return _vars[0].names().size();
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
SystemBase::hasVector(TagID tag)
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
SystemBase::hasMatrix(TagID tag)
{
  return tag < _tagged_matrices.size() && _tagged_matrices[tag];
}

SparseMatrix<Number> &
SystemBase::getMatrix(TagID tag)
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
SystemBase::matrixTagActive(TagID tag)
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
    if (getVariable(0, vci._dest_name).isNodal())
      io.copy_nodal_solution(system(), vci._dest_name, vci._source_name, timestep);
    else
      io.copy_elemental_solution(system(), vci._dest_name, vci._source_name, timestep);
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

template MooseVariableFE<Real> & SystemBase::getFieldVariable<Real>(THREAD_ID tid,
                                                                    const std::string & var_name);

template MooseVariableFE<RealVectorValue> &
SystemBase::getFieldVariable<RealVectorValue>(THREAD_ID tid, const std::string & var_name);

template MooseVariableFE<Real> & SystemBase::getFieldVariable<Real>(THREAD_ID tid,
                                                                    unsigned int var_number);

template MooseVariableFE<RealVectorValue> &
SystemBase::getFieldVariable<RealVectorValue>(THREAD_ID tid, unsigned int var_number);
