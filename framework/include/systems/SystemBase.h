//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <vector>

#include "DataIO.h"
#include "MooseTypes.h"
#include "VariableWarehouse.h"
#include "InputParameters.h"
#include "MooseObjectWarehouseBase.h"
#include "MooseVariableBase.h"
#include "ConsoleStreamInterface.h"

// libMesh
#include "libmesh/exodusII_io.h"
#include "libmesh/parallel_object.h"
#include "libmesh/numeric_vector.h"
#include "libmesh/sparse_matrix.h"

// Forward declarations
class Factory;
class MooseApp;
class MooseVariableFieldBase;
template <typename>
class MooseVariableFE;
typedef MooseVariableFE<Real> MooseVariable;
typedef MooseVariableFE<VectorValue<Real>> VectorMooseVariable;
class MooseMesh;
class SubProblem;
class SystemBase;
class TimeIntegrator;
class InputParameters;

// libMesh forward declarations
namespace libMesh
{
class System;
class DofMap;
class FEType;
}

/**
 * ///< Type of coordinate system
 */
void extraSendList(std::vector<dof_id_type> & send_list, void * context);

/**
 * Free function used for a libMesh callback
 */
void extraSparsity(SparsityPattern::Graph & sparsity,
                   std::vector<dof_id_type> & n_nz,
                   std::vector<dof_id_type> & n_oz,
                   void * context);

/**
 * IO Methods for restart, backup and restore.
 */
template <>
void dataStore(std::ostream & stream, SystemBase & system_base, void * context);

/**
 * IO Methods for restart, backup and restore.
 */
template <>
void dataLoad(std::istream & stream, SystemBase & system_base, void * context);

/**
 * Information about variables that will be copied
 */
struct VarCopyInfo
{
  VarCopyInfo(const std::string & dest_name,
              const std::string & source_name,
              const std::string & timestep)
    : _dest_name(dest_name), _source_name(source_name), _timestep(timestep)
  {
  }

  std::string _dest_name;
  std::string _source_name;
  std::string _timestep;
};

/**
 * Base class for a system (of equations)
 *
 */
class SystemBase : public libMesh::ParallelObject, public ConsoleStreamInterface

{
public:
  SystemBase(SubProblem & subproblem, const std::string & name, Moose::VarKindType var_kind);
  virtual ~SystemBase() {}

  /**
   * Gets the number of this system
   * @return The number of this system
   */
  virtual unsigned int number() const;
  virtual MooseMesh & mesh() { return _mesh; }
  virtual const MooseMesh & mesh() const { return _mesh; }
  virtual SubProblem & subproblem() { return _subproblem; }
  virtual const SubProblem & subproblem() const { return _subproblem; }

  /**
   * Applies scaling factors to the system's variables
   * @param inverse_scaling_factors A vector containing the inverse of each variable's scaling
   * factor, e.g. 1 / scaling_factor
   */
  void applyScalingFactors(const std::vector<Real> & inverse_scaling_factors);

  /**
   * Whether we are computing an initial Jacobian for automatic variable scaling
   */
  bool computingScalingJacobian() const { return _computing_scaling_jacobian; }

  /**
   * Setter for whether we're computing the scaling jacobian
   */
  void computingScalingJacobian(bool computing_scaling_jacobian)
  {
    _computing_scaling_jacobian = computing_scaling_jacobian;
  }

  /**
   * Whether we are computing an initial Residual for automatic variable scaling
   */
  bool computingScalingResidual() const { return _computing_scaling_residual; }

  /**
   * Setter for whether we're computing the scaling residual
   */
  void computingScalingResidual(bool computing_scaling_residual)
  {
    _computing_scaling_residual = computing_scaling_residual;
  }

  /**
   * Getter for whether we are performing automatic scaling
   * @return whether we are performing automatic scaling
   */
  bool automaticScaling() const { return _automatic_scaling; }

  /**
   * Setter for whether we are performing automatic scaling
   * @param automatic_scaling A boolean representing whether we are performing automatic scaling
   */
  void automaticScaling(bool automatic_scaling) { _automatic_scaling = automatic_scaling; }

  /**
   * Sets the verbose flag
   * @param[in] verbose   Verbose flag
   */
  void setVerboseFlag(const bool & verbose) { _verbose = verbose; }

  /**
   * Gets writeable reference to the dof map
   */
  virtual DofMap & dofMap();

  /**
   * Gets const reference to the dof map
   */
  virtual const DofMap & dofMap() const;

  /**
   * Get the reference to the libMesh system
   */
  virtual System & system() = 0;
  virtual const System & system() const = 0;

  /**
   * Initialize the system
   */
  virtual void init(){};

  /**
   * Called only once, just before the solve begins so objects can do some precalculations
   */
  virtual void initializeObjects(){};

  /**
   * Method called during initialSetup to add extra system vector if they are required by
   * the simulation
   */
  virtual void addExtraVectors();

  /**
   * Update the system (doing libMesh magic)
   */
  virtual void update();

  /**
   * Solve the system (using libMesh magic)
   */
  virtual void solve();

  virtual void copyOldSolutions();
  virtual void restoreSolutions();

  /**
   * The solution vector that is currently being operated on.
   * This is typically a ghosted vector that comes in from the Nonlinear solver.
   */
  virtual const NumericVector<Number> * const & currentSolution() const = 0;

  NumericVector<Number> & solution() { return solutionState(0); }
  NumericVector<Number> & solutionOld() { return solutionState(1); }
  NumericVector<Number> & solutionOlder() { return solutionState(2); }
  const NumericVector<Number> & solution() const { return solutionState(0); }
  const NumericVector<Number> & solutionOld() const { return solutionState(1); }
  const NumericVector<Number> & solutionOlder() const { return solutionState(2); }

  virtual const NumericVector<Number> * solutionPreviousNewton() const = 0;
  virtual NumericVector<Number> * solutionPreviousNewton() = 0;

  /**
   * Get a state of the solution (0 = current, 1 = old, 2 = older, etc).
   *
   * If the state does not exist, it will be initialized in addition to any newer
   * states before it that have not been initialized.
   */
  NumericVector<Number> & solutionState(const unsigned int state);

  /**
   * Get a state of the solution (0 = current, 1 = old, 2 = older, etc).
   *
   * By default, up to state _default_solution_states is added. Any older states must be
   * added using the non-const solutionState().
   */
  const NumericVector<Number> & solutionState(const unsigned int state) const;

  virtual Number & duDotDu() { return _du_dot_du; }
  virtual Number & duDotDotDu() { return _du_dotdot_du; }
  virtual const Number & duDotDu() const { return _du_dot_du; }
  virtual const Number & duDotDotDu() const { return _du_dotdot_du; }

  // non-const getters
  virtual NumericVector<Number> * solutionUDot() = 0;
  virtual NumericVector<Number> * solutionUDotOld() = 0;
  virtual NumericVector<Number> * solutionUDotDot() = 0;
  virtual NumericVector<Number> * solutionUDotDotOld() = 0;
  // const getters
  virtual const NumericVector<Number> * solutionUDot() const = 0;
  virtual const NumericVector<Number> * solutionUDotOld() const = 0;
  virtual const NumericVector<Number> * solutionUDotDot() const = 0;
  virtual const NumericVector<Number> * solutionUDotDotOld() const = 0;

  virtual void saveOldSolutions();
  virtual void restoreOldSolutions();

  /**
   * Check if the named vector exists in the system.
   */
  bool hasVector(const std::string & tag_name) const;

  /**
   * Check if the tagged vector exists in the system.
   */
  virtual bool hasVector(TagID tag_id) const;

  /**
   * Ideally, we should not need this API.
   * There exists a really bad API "addCachedResidualDirectly " in FEProblem and DisplacedProblem
   * This API should go away once addCachedResidualDirectly is removed in the future
   * Return Tag ID for Time
   */
  virtual TagID timeVectorTag();

  /**
   * Return the Matrix Tag ID for Time
   */
  virtual TagID timeMatrixTag();

  /**
   * Return the Matrix Tag ID for System
   */
  virtual TagID systemMatrixTag();

  /*
   * Return TagID for nontime
   */
  virtual TagID nonTimeVectorTag();

  /*
   * Return TagID for nontime
   */
  virtual TagID residualVectorTag();

  /**
   * Get a raw NumericVector
   */
  virtual NumericVector<Number> & getVector(const std::string & name);

  /**
   * Get a raw NumericVector
   */
  virtual NumericVector<Number> & getVector(TagID tag);

  /**
   * Get a raw NumericVector
   */
  virtual const NumericVector<Number> & getVector(TagID tag) const;

  /**
   * Associate a vector for a given tag
   */
  virtual void associateVectorToTag(NumericVector<Number> & vec, TagID tag);

  /**
   * Associate a vector for a given tag
   */
  virtual void disassociateVectorFromTag(NumericVector<Number> & vec, TagID tag);

  /**
   * Disassociate all vectors, and then hasVector() will return false.
   */
  virtual void disassociateAllTaggedVectors();

  /**
   * Check if the tagged matrix exists in the system.
   */
  virtual bool hasMatrix(TagID tag) const;

  /**
   * Get a raw SparseMatrix
   */
  virtual SparseMatrix<Number> & getMatrix(TagID tag);

  /**
   * Get a raw SparseMatrix
   */
  virtual const SparseMatrix<Number> & getMatrix(TagID tag) const;

  /**
   *  Make all exsiting matrices ative
   */
  virtual void activeAllMatrixTags();

  /**
   *  Active a matrix for tag
   */
  virtual void activeMatrixTag(TagID tag);

  /**
   *  If or not a matrix tag is active
   */
  virtual bool matrixTagActive(TagID tag) const;

  /**
   *  deactive a matrix for tag
   */
  virtual void deactiveMatrixTag(TagID tag);

  /**
   * Make matrices inactive
   */
  virtual void deactiveAllMatrixTags();

  /**
   * Close all matrices associated the tags
   */
  void closeTaggedMatrices(const std::set<TagID> & tags);

  /**
   * associate a matirx to a tag
   */
  virtual void associateMatrixToTag(SparseMatrix<Number> & matrix, TagID tag);

  /**
   * disassociate a matirx from a tag
   */
  virtual void disassociateMatrixFromTag(SparseMatrix<Number> & matrix, TagID tag);

  /**
   * Clear all tagged matrices
   */
  virtual void disassociateAllTaggedMatrices();

  /**
   * Returns a reference to a serialized version of the solution vector for this subproblem
   */
  virtual NumericVector<Number> & serializedSolution() = 0;

  virtual NumericVector<Number> & residualCopy()
  {
    mooseError("This system does not support getting a copy of the residual");
  }
  virtual NumericVector<Number> & residualGhosted()
  {
    mooseError("This system does not support getting a ghosted copy of the residual");
  }

  /**
   * Will modify the send_list to add all of the extra ghosted dofs for this system
   */
  virtual void augmentSendList(std::vector<dof_id_type> & send_list);

  /**
   * Will modify the sparsity pattern to add logical geometric connections
   */
  virtual void augmentSparsity(SparsityPattern::Graph & sparsity,
                               std::vector<dof_id_type> & n_nz,
                               std::vector<dof_id_type> & n_oz) = 0;

  /**
   * Canonical method for adding a variable
   * @param var_type the type of the variable, e.g. MooseVariableScalar
   * @param var_name the variable name, e.g. 'u'
   * @param params the InputParameters from which to construct the variable
   */
  virtual void addVariable(const std::string & var_type,
                           const std::string & var_name,
                           InputParameters & parameters);

  /**
   * If a variable is an array variable
   */
  virtual bool isArrayVariable(const std::string & var_name) const;

  ///@{
  /**
   * Query a system for a variable
   *
   * @param var_name name of the variable
   * @return true if the variable exists
   */
  virtual bool hasVariable(const std::string & var_name) const;
  virtual bool hasScalarVariable(const std::string & var_name) const;
  ///@}

  virtual bool isScalarVariable(unsigned int var_name) const;

  /**
   * Gets a reference to a variable of with specified name
   *
   * @param tid Thread id
   * @param var_name variable name
   * @return reference the variable (class)
   */
  MooseVariableFEBase & getVariable(THREAD_ID tid, const std::string & var_name);

  /**
   * Gets a reference to a variable with specified number
   *
   * @param tid Thread id
   * @param var_number libMesh variable number
   * @return reference the variable (class)
   */
  MooseVariableFEBase & getVariable(THREAD_ID tid, unsigned int var_number);

  /**
   * Gets a reference to a variable of with specified name
   *
   * This excludes and cannot return finite volume variables.
   *
   * @param tid Thread id
   * @param var_name variable name
   * @return reference the variable (class)
   */
  template <typename T>
  MooseVariableFE<T> & getFieldVariable(THREAD_ID tid, const std::string & var_name);

  /**
   * Returns a field variable pointer - this includes finite volume variables.
   */
  template <typename T>
  MooseVariableField<T> & getActualFieldVariable(THREAD_ID tid, const std::string & var_name);

  /**
   * Gets a reference to a variable with specified number
   *
   * This excludes and cannot return finite volume variables.
   *
   * @param tid Thread id
   * @param var_number libMesh variable number
   * @return reference the variable (class)
   */
  template <typename T>
  MooseVariableFE<T> & getFieldVariable(THREAD_ID tid, unsigned int var_number);

  /**
   * Returns a field variable pointer - this includes finite volume variables.
   */
  template <typename T>
  MooseVariableField<T> & getActualFieldVariable(THREAD_ID tid, unsigned int var_number);

  /**
   * Gets a reference to a scalar variable with specified number
   *
   * @param tid Thread id
   * @param var_name A string which is the name of the variable to get.
   * @return reference the variable (class)
   */
  virtual MooseVariableScalar & getScalarVariable(THREAD_ID tid, const std::string & var_name);

  /**
   * Gets a reference to a variable with specified number
   *
   * @param tid Thread id
   * @param var_number libMesh variable number
   * @return reference the variable (class)
   */
  virtual MooseVariableScalar & getScalarVariable(THREAD_ID tid, unsigned int var_number);

  /**
   * Get the block where a variable of this system is defined
   *
   * @param var_number The number of the variable
   * @return the set of subdomain ids where the variable is active (defined)
   */
  virtual const std::set<SubdomainID> * getVariableBlocks(unsigned int var_number);

  /**
   * Get the number of variables in this system
   * @return the number of variables
   */
  virtual unsigned int nVariables() const;

  /**
   * Gets the maximum number of dofs used by any one variable on any one element
   *
   * @return The max
   */
  std::size_t getMaxVarNDofsPerElem() const { return _max_var_n_dofs_per_elem; }

  /**
   * Gets the maximum number of dofs used by any one variable on any one node
   *
   * @return The max
   */
  std::size_t getMaxVarNDofsPerNode() const { return _max_var_n_dofs_per_node; }

  /**
   * assign the maximum element dofs
   */
  void assignMaxVarNDofsPerElem(std::size_t max_dofs) { _max_var_n_dofs_per_elem = max_dofs; }

  /**
   * assign the maximum node dofs
   */
  void assignMaxVarNDofsPerNode(std::size_t max_dofs) { _max_var_n_dofs_per_node = max_dofs; }

  /**
   * Adds this variable to the list of variables to be zeroed during each residual evaluation.
   * @param var_name The name of the variable to be zeroed.
   */
  virtual void addVariableToZeroOnResidual(std::string var_name);

  /**
   * Adds this variable to the list of variables to be zeroed during each Jacobian evaluation.
   * @param var_name The name of the variable to be zeroed.
   */
  virtual void addVariableToZeroOnJacobian(std::string var_name);

  /**
   * Zero out the solution for the list of variables passed in.
   *
   * @ param vars_to_be_zeroed The variable names in this vector will have their solutions set to
   * zero after this call
   */
  virtual void zeroVariables(std::vector<std::string> & vars_to_be_zeroed);

  /**
   * Zero out the solution for the variables that were registered as needing to have their solutions
   * zeroed on out on residual evaluation by a call to addVariableToZeroOnResidual()
   */
  virtual void zeroVariablesForResidual();

  /**
   * Zero out the solution for the variables that were registered as needing to have their solutions
   * zeroed on out on Jacobian evaluation by a call to addVariableToZeroOnResidual()
   */
  virtual void zeroVariablesForJacobian();

  /**
   * Get minimal quadrature order needed for integrating variables in this system
   * @return The minimal order of quadrature
   */
  virtual Order getMinQuadratureOrder();

  /**
   * Prepare the system for use
   * @param tid ID of the thread
   */
  virtual void prepare(THREAD_ID tid);

  /**
   * Prepare the system for use on sides
   *
   * This will try to reuse the preparation done on the element.
   *
   * @param tid ID of the thread
   * @param resize_data Pass True if this system needs to resize residual and jacobian
   * datastructures based on preparing this face
   */
  virtual void prepareFace(THREAD_ID tid, bool resize_data);

  /**
   * Prepare the system for use
   * @param tid ID of the thread
   */
  virtual void prepareNeighbor(THREAD_ID tid);

  /**
   * Prepare the system for use for lower dimensional elements
   * @param tid ID of the thread
   */
  virtual void prepareLowerD(THREAD_ID tid);

  /**
   * Reinit an element assembly info
   * @param elem Which element we are reinitializing for
   * @param tid ID of the thread
   */
  virtual void reinitElem(const Elem * elem, THREAD_ID tid);

  /**
   * Reinit assembly info for a side of an element
   * @param elem The element
   * @param side Side of of the element
   * @param bnd_id Boundary id on that side
   * @param tid Thread ID
   */
  virtual void
  reinitElemFace(const Elem * elem, unsigned int side, BoundaryID bnd_id, THREAD_ID tid);

  /**
   * Compute the values of the variables at all the current points.
   */
  virtual void
  reinitNeighborFace(const Elem * elem, unsigned int side, BoundaryID bnd_id, THREAD_ID tid);

  /**
   * Compute the values of the variables at all the current points.
   */
  virtual void reinitNeighbor(const Elem * elem, THREAD_ID tid);

  /**
   * Compute the values of the variables on the lower dimensional element
   */
  virtual void reinitLowerD(THREAD_ID tid);

  /**
   * Reinit nodal assembly info
   * @param node Node to reinit for
   * @param tid Thread ID
   */
  virtual void reinitNode(const Node * node, THREAD_ID tid);

  /**
   * Reinit nodal assembly info on a face
   * @param node Node to reinit
   * @param bnd_id Boundary ID
   * @param tid Thread ID
   */
  virtual void reinitNodeFace(const Node * node, BoundaryID bnd_id, THREAD_ID tid);

  /**
   * Reinit variables at a set of nodes
   * @param nodes List of node ids to reinit
   * @param tid Thread ID
   */
  virtual void reinitNodes(const std::vector<dof_id_type> & nodes, THREAD_ID tid);

  /**
   * Reinit variables at a set of neighbor nodes
   * @param nodes List of node ids to reinit
   * @param tid Thread ID
   */
  virtual void reinitNodesNeighbor(const std::vector<dof_id_type> & nodes, THREAD_ID tid);

  /**
   * Reinit scalar varaibles
   * @param tid Thread ID
   * @param reinit_for_derivative_reordering A flag indicating whether we are reinitializing for the
   *        purpose of re-ordering derivative information for ADNodalBCs
   */
  virtual void reinitScalars(THREAD_ID tid, bool reinit_for_derivative_reordering = false);

  /**
   * Add info about variable that will be copied
   *
   * @param dest_name Name of the nodal variable being used for copying into (name is from the
   * exodusII file)
   * @param source_name Name of the nodal variable being used for copying from (name is from the
   * exodusII file)
   * @param timestep Timestep in the file being used
   */
  virtual void addVariableToCopy(const std::string & dest_name,
                                 const std::string & source_name,
                                 const std::string & timestep);

  const std::vector<MooseVariableFEBase *> & getVariables(THREAD_ID tid)
  {
    return _vars[tid].fieldVariables();
  }

  const std::vector<MooseVariableScalar *> & getScalarVariables(THREAD_ID tid)
  {
    return _vars[tid].scalars();
  }

  const std::set<SubdomainID> & getSubdomainsForVar(unsigned int var_number) const
  {
    return _var_map.at(var_number);
  }

  /**
   * Remove a vector from the system with the given name.
   */
  virtual void removeVector(const std::string & name);

  /**
   * Adds a solution length vector to the system.
   *
   * @param vector_name The name of the vector.
   * @param project Whether or not to project this vector when doing mesh refinement.
   *                If the vector is just going to be recomputed then there is no need to project
   * it.
   * @param type What type of parallel vector.  This is usually either PARALLEL or GHOSTED.
   *                                            GHOSTED is needed if you are going to be accessing
   * off-processor entries.
   *                                            The ghosting pattern is the same as the solution
   * vector.
   */
  virtual NumericVector<Number> &
  addVector(const std::string & vector_name, const bool project, const ParallelType type);

  /**
   * Adds a solution length vector to the system with the specified TagID
   *
   * @param tag_name The name of the tag
   * @param project Whether or not to project this vector when doing mesh refinement.
   *                If the vector is just going to be recomputed then there is no need to project
   * it.
   * @param type What type of parallel vector.  This is usually either PARALLEL or GHOSTED.
   *                                            GHOSTED is needed if you are going to be accessing
   * off-processor entries.
   *                                            The ghosting pattern is the same as the solution
   * vector.
   */
  NumericVector<Number> & addVector(TagID tag, const bool project, const ParallelType type);

  /**
   * Close all vectors for given tags
   */
  virtual void closeTaggedVectors(const std::set<TagID> & tags);

  /**
   * Zero all vectors for given tags
   */
  virtual void zeroTaggedVectors(const std::set<TagID> & tags);

  /**
   * Remove a solution length vector from the system with the specified TagID
   *
   * @param tag_id  Tag ID
   */
  virtual void removeVector(TagID tag_id);

  /**
   * Adds a jacobian sized vector
   *
   * @param tag_name The name of the tag
   */
  virtual SparseMatrix<Number> & addMatrix(TagID /* tag */)
  {
    mooseError("Adding a matrix is not supported for this type of system!");
  }

  /**
   * Removes a jacobian sized vector
   *
   * @param tag_name The name of the tag
   */
  virtual void removeMatrix(TagID /* tag */)
  {
    mooseError("Removing a matrix is not supported for this type of system!");
  }

  virtual const std::string & name() const;

  const std::vector<VariableName> & getVariableNames() const { return _vars[0].names(); }

  void getStandardFieldVariableNames(std::vector<VariableName> & std_field_variables) const;

  /**
   * Returns the maximum number of all variables on the system
   */
  unsigned int getMaxVariableNumber() const { return _max_var_number; }

  virtual void computeVariables(const NumericVector<Number> & /*soln*/) {}

  void copyVars(ExodusII_IO & io);

  /**
   * Copy current solution into old and older
   */
  virtual void copySolutionsBackwards();

  virtual void addTimeIntegrator(const std::string & /*type*/,
                                 const std::string & /*name*/,
                                 InputParameters & /*parameters*/)
  {
  }

  virtual void addTimeIntegrator(std::shared_ptr<TimeIntegrator> /*ti*/) {}

  TimeIntegrator * getTimeIntegrator() { return _time_integrator.get(); }
  const TimeIntegrator * getTimeIntegrator() const { return _time_integrator.get(); }

  std::shared_ptr<TimeIntegrator> getSharedTimeIntegrator() { return _time_integrator; }

  /// caches the dof indices of provided variables in MooseMesh's FaceInfo data structure
  void cacheVarIndicesByFace(const std::vector<VariableName> & vars);

  /// Whether or not there are variables to be restarted from an Exodus mesh file
  bool hasVarCopy() const { return _var_to_copy.size() > 0; }

protected:
  /**
   * Internal getters for the states of the solution as owned by libMesh.
   *
   * For the first three states (0 = current, 1 = old, 2 = older), we point directly to the
   * solutions in libMesh (which is why these virtuals are needed). This allows us to store a more
   * generalized set of solution states in _solution_states that also enables the addition of older
   * states if we need them.
   */
  ///@{
  virtual NumericVector<Number> & solutionInternal() const = 0;
  virtual NumericVector<Number> & solutionOldInternal() const = 0;
  virtual NumericVector<Number> & solutionOlderInternal() const = 0;
  ///@}

  SubProblem & _subproblem;

  MooseApp & _app;
  Factory & _factory;

  MooseMesh & _mesh;
  /// The name of this system
  std::string _name;

  /// Variable warehouses (one for each thread)
  std::vector<VariableWarehouse> _vars;
  /// Map of variables (variable id -> array of subdomains where it lives)
  std::map<unsigned int, std::set<SubdomainID>> _var_map;
  /// Maximum variable number
  unsigned int _max_var_number;

  std::vector<std::string> _vars_to_be_zeroed_on_residual;
  std::vector<std::string> _vars_to_be_zeroed_on_jacobian;

  Real _du_dot_du;
  Real _du_dotdot_du;

  /// Tagged vectors (pointer)
  std::vector<NumericVector<Number> *> _tagged_vectors;
  /// Tagged matrices (pointer)
  std::vector<SparseMatrix<Number> *> _tagged_matrices;
  /// Active flags for tagged matrices
  std::vector<bool> _matrix_tag_active_flags;

  // Used for saving old solutions so that they wont be accidentally changed
  NumericVector<Real> * _saved_old;
  NumericVector<Real> * _saved_older;

  // Used for saving old u_dot and u_dotdot so that they wont be accidentally changed
  NumericVector<Real> * _saved_dot_old;
  NumericVector<Real> * _saved_dotdot_old;

  /// default kind of variables in this system
  Moose::VarKindType _var_kind;

  std::vector<VarCopyInfo> _var_to_copy;

  /// Maximum number of dofs for any one variable on any one element
  size_t _max_var_n_dofs_per_elem;

  /// Maximum number of dofs for any one variable on any one node
  size_t _max_var_n_dofs_per_node;

  /// Time integrator
  std::shared_ptr<TimeIntegrator> _time_integrator;

  /// Map variable number to its pointer
  std::vector<std::vector<MooseVariableFEBase *>> _numbered_vars;

  /// Storage for MooseVariable objects
  MooseObjectWarehouseBase<MooseVariableBase> _variable_warehouse;

  /// Flag used to indicate whether we are computing the scaling Jacobian
  bool _computing_scaling_jacobian;

  /// Flag used to indicate whether we are computing the scaling Residual
  bool _computing_scaling_residual;

  /// Whether to automatically scale the variables
  bool _automatic_scaling;

  /// True if printing out additional information
  bool _verbose;

  /// The number of default solution states to store
  unsigned int _default_solution_states;

private:
  /// The solution states (0 = current, 1 = old, 2 = older, etc)
  std::vector<NumericVector<Number> *> _solution_states;
  /// The saved solution states (0 = current, 1 = old, 2 = older, etc)
  std::vector<NumericVector<Number> *> _saved_solution_states;
};

#define PARALLEL_TRY

#define PARALLEL_CATCH _fe_problem.checkExceptionAndStopSolve();
