//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Action.h"
#include "ActionWarehouse.h"
#include "InputParametersChecksUtils.h"
#include "ActionComponent.h"

// We include these headers for all the derived classes that will be building objects
#include "FEProblemBase.h"
#include "Factory.h"
#include "MultiMooseEnum.h"

#define registerPhysicsBaseTasks(app_name, derived_name)                                           \
  registerMooseAction(app_name, derived_name, "init_physics");                                     \
  registerMooseAction(app_name, derived_name, "copy_vars_physics");                                \
  registerMooseAction(app_name, derived_name, "check_integrity_early_physics")

/**
 * Base class to help creating an entire physics
 */
class PhysicsBase : public Action, public InputParametersChecksUtils<PhysicsBase>
{
public:
  static InputParameters validParams();

  PhysicsBase(const InputParameters & parameters);

  /// Provide additional parameters for the relationship managers
  virtual InputParameters getAdditionalRMParams() const { return emptyInputParameters(); };

  // Responding to tasks //
  /// Forwards from the action tasks to the implemented addXYZ() in the derived classes
  /// If you need more than these:
  /// - register your action to the new task using
  ///   registerMooseAction("AppName", ActionClass, "task_name");
  /// - override actOnAdditionalTasks and add your additional work there
  virtual void act() override final;

  /// Routine to add additional setup work on additional registered tasks to a Physics
  virtual void actOnAdditionalTasks() {}

  // Block restriction //
  /**
   * @brief Add new blocks to the Physics
   * @param blocks list of blocks to add to the physics
   */
  void addBlocks(const std::vector<SubdomainName> & blocks);
  void addBlocksById(const std::vector<SubdomainID> & block_ids);

  /// Return the blocks this physics is defined on
  const std::vector<SubdomainName> & blocks() const { return _blocks; }

  /**
   * @brief Check if an external object has the same block restriction
   * @param object_name name of the object to check the block restriction of
   * @param blocks the blocks for this object
   * @param error_if_not_identical whether to error if the block restrictions dont match
   */
  bool checkBlockRestrictionIdentical(const std::string & object_name,
                                      const std::vector<SubdomainName> & blocks,
                                      const bool error_if_not_identical = true) const;

  // Coupling with Physics //
  /**
   * @brief Get a Physics from the ActionWarehouse with the requested type and name
   * @param phys_name name of the Physics to retrieve
   * @param allow_fail whether to allow returning a nullptr if the physics does not exist
   */
  template <typename T>
  const T * getCoupledPhysics(const PhysicsName & phys_name, const bool allow_fail = false) const;
  /// Get all Physics from the ActionWarehouse with the requested type
  template <typename T>
  const std::vector<T *> getCoupledPhysics(const bool allow_fail = false) const;

  /// Return the maximum dimension of the blocks the Physics is active on
  unsigned int dimension() const;

  // Coupling with Components //
  /// Get a component with the requested name
  const ActionComponent & getActionComponent(const ComponentName & comp_name) const;
  /// Check that the component is of the desired type
  template <typename T>
  void checkComponentType(const ActionComponent & component) const;
  /// Most basic way of adding a component: simply adding the blocks to the block
  /// restriction of the Physics. More complex behavior should be implemented by overriding
  virtual void addComponent(const ActionComponent & component);

  /// Return the list of solver (nonlinear + linear) variables in this physics
  const std::vector<VariableName> & solverVariableNames() const { return _solver_var_names; };
  /// Return the list of aux variables in this physics
  const std::vector<VariableName> & auxVariableNames() const { return _aux_var_names; };

protected:
  /// Return whether the Physics is solved using a transient
  bool isTransient() const;

  /// Get the factory for this physics
  /// The factory lets you get the parameters for objects
  Factory & getFactory() { return _factory; }
  Factory & getFactory() const { return _factory; }
  /// Get the problem for this physics
  /// Useful to add objects to the simulation
  virtual FEProblemBase & getProblem()
  {
    mooseAssert(_problem, "Requesting the problem too early");
    return *_problem;
  }
  virtual const FEProblemBase & getProblem() const
  {
    mooseAssert(_problem, "Requesting the problem too early");
    return *_problem;
  }

  /// Tell the app if we want to use Exodus restart
  void prepareCopyVariablesFromMesh() const;
  /**
   * Copy nonlinear or aux variables from the mesh file
   *
   * @param variables_to_copy  Nonlinear or aux (not a mix) variables
   * @param are_nonlinear  True if \c variables_to_copy are nonlinear; else, aux
   */
  void copyVariablesFromMesh(const std::vector<VariableName> & variables_to_copy,
                             bool are_nonlinear = true);

  /// Use prefix() to disambiguate names
  std::string prefix() const { return name() + "_"; }

  /// Keep track of the name of the solver variable defined in the Physics
  void saveSolverVariableName(const VariableName & var_name)
  {
    _solver_var_names.push_back(var_name);
  }
  /// Keep track of the name of an aux variable defined in the Physics
  void saveAuxVariableName(const VariableName & var_name) { _aux_var_names.push_back(var_name); }

  /// Check whether a variable already exists
  bool variableExists(const VariableName & var_name, bool error_if_aux) const;
  /// Check whether a variable already exists and is a solver variable
  bool solverVariableExists(const VariableName & var_name) const;

  /// Get the solver system for this variable index. The index should be the index of the variable in solver
  /// var_names (currently _solver_var_names) vector
  const SolverSystemName & getSolverSystem(unsigned int variable_index) const;
  /// Get the solver system for this variable name
  const SolverSystemName & getSolverSystem(const VariableName & variable_name) const;

  /// Add a new required task for all physics deriving from this class
  /// NOTE: This does not register the task, you still need to call registerMooseAction
  void addRequiredPhysicsTask(const std::string & task) { _required_tasks.insert(task); }

  /**
   * @brief Set the blocks parameter to the input parameters of an object this Physics will create
   * @param params the parameters of the object
   * @param blocks the blocks to set as the parameter
   */
  void assignBlocks(InputParameters & params, const std::vector<SubdomainName> & blocks) const;
  /**
   * @brief Check if a vector contains all the mesh blocks
   * @param blocks the vector blocks to check for whether it contains every block in the mesh
   */
  bool allMeshBlocks(const std::vector<SubdomainName> & blocks) const;
  bool allMeshBlocks(const std::set<SubdomainName> & blocks) const;
  // These APIs can deal with ANY_BLOCK_ID or ids with no names. They will be slower than the
  // MooseMeshUtils' APIs, but are more convenient for setup purposes
  /// Get the set of subdomain ids for the incoming vector of subdomain names
  std::set<SubdomainID> getSubdomainIDs(const std::set<SubdomainName> & blocks) const;
  /// Get the vector of subdomain names and ids for the incoming set of subdomain IDs
  std::vector<std::string> getSubdomainNamesAndIDs(const std::set<SubdomainID> & blocks) const;

  /**
   * Process the given petsc option pairs into the system solver settings
   */
  void addPetscPairsToPetscOptions(
      const std::vector<std::pair<MooseEnumItem, std::string>> & petsc_pair_options);

  // Helpers to check on variable types
  /// Whether the variable is a finite volume variable
  bool isVariableFV(const VariableName & var_name) const;
  /// Whether the variable is a scalar variable (global single scalar, not a field)
  bool isVariableScalar(const VariableName & var_name) const;

  // Routines to help with deciding when to create objects
  /**
   * Returns whether this Physics should create the variable. Will return false if the variable
   * already exists and has the necessary block restriction.
   * @param var_name name of the variable
   * @param blocks block restriction to use. If empty, no block restriction
   * @param error_if_aux error if the variable is auxiliary
   */
  bool shouldCreateVariable(const VariableName & var_name,
                            const std::vector<SubdomainName> & blocks,
                            const bool error_if_aux);

  /**
   * Returns whether this Physics should create the variable. Will return false if the initial
   * condition already exists and has the necessary block restriction.
   * @param var_name name of the variable
   * @param blocks block restriction to use. If empty, no block restriction
   * @param ic_is_default_ic whether this IC is from a default parameter, and therefore should be
   * skipped when recovering/restarting
   * @param error_if_already_defined two ICs cannot be defined on the same subdomain, so if this is
   * set to true, any overlap between the subdomains of two ICs for the same variable will cause an
   * error. If set to false, the existing ICs will take priority, and this routine will return
   * false. Setting 'error_if_already_defined' to '!ic_is_default_ic' is a good idea if it is ok to
   * overwrite the default IC value of the Physics.
   */
  bool shouldCreateIC(const VariableName & var_name,
                      const std::vector<SubdomainName> & blocks,
                      const bool ic_is_default_ic,
                      const bool error_if_already_defined) const;

  /**
   * Returns whether this Physics should create the variable. Will return false if the time
   * derivative kernel already exists and has the necessary block restriction.
   * @param var_name name of the variable
   * @param blocks block restriction to use. If empty, no block restriction
   * @param error_if_already_defined two time derivatives can be defined on the same subdomain, but
   * it is usually not correct. So if this is set to true, any overlap between the subdomains of two
   * time derivatives for the same variable will cause an error. If set to false, the existing time
   * derivative will be deemed as sufficient, and this routine will return false.
   */
  bool shouldCreateTimeDerivative(const VariableName & var_name,
                                  const std::vector<SubdomainName> & blocks,
                                  const bool error_if_already_defined) const;

  // Other conceivable "shouldCreate" routines for things that are unique to a variable
  // - shouldCreateTimeIntegrator
  // - shouldCreatePredictor/Corrector

  /**
   * When this is called, we are knowingly not using the value of these parameters. This routine
   * checks whether these parameters simply have defaults or were passed by the user
   * @param param_names the parameters we are ignoring
   * @param object_type the type of the object we are not building (thus ignoring the parameters)
   * @param object_name the name of the object we are not building
   */
  void reportPotentiallyMissedParameters(const std::vector<std::string> & param_names,
                                         const std::string & object_type,
                                         const std::string & object_name = "") const;

  /// System names for the system(s) owning the solver variables
  std::vector<SolverSystemName> _system_names;

  /// System numbers for the system(s) owning the solver variables
  std::vector<unsigned int> _system_numbers;

  /// Whether to output additional information
  const bool _verbose;

  /// Whether to add a default preconditioning.
  /// The implementation of the default is defined by the derived class
  const MooseEnum & _preconditioning;

  /// Keep track of the subdomains the Physics is defined on
  std::vector<SubdomainName> _blocks;

private:
  /// Gathers additional parameters for the relationship managers from the Physics
  /// then calls the parent Action::addRelationshipManagers with those parameters
  using Action::addRelationshipManagers;
  virtual void addRelationshipManagers(Moose::RelationshipManagerType input_rm_type) override;

  /// Process some parameters that require the problem to be created. Executed on init_physics
  void initializePhysics();
  /// Additional initialization work that should happen very early, as soon as the problem is created
  virtual void initializePhysicsAdditional() {}
  /// Additional checks performed once the executioner / executor has been created
  virtual void checkIntegrityEarly() const;
  /// Additional checks performed near the end of the setup phase
  virtual void checkIntegrity() const {}

  /// The default implementation of these routines will do nothing as we do not expect all Physics
  /// to be defining an object of every type
  virtual void addSolverVariables() {}
  virtual void addAuxiliaryVariables() {}
  virtual void addInitialConditions() {}
  virtual void addFEKernels() {}
  virtual void addFVKernels() {}
  virtual void addNodalKernels() {}
  virtual void addDiracKernels() {}
  virtual void addDGKernels() {}
  virtual void addScalarKernels() {}
  virtual void addInterfaceKernels() {}
  virtual void addFVInterfaceKernels() {}
  virtual void addFEBCs() {}
  virtual void addFVBCs() {}
  virtual void addNodalBCs() {}
  virtual void addPeriodicBCs() {}
  virtual void addFunctions() {}
  virtual void addAuxiliaryKernels() {}
  virtual void addMaterials() {}
  virtual void addFunctorMaterials() {}
  virtual void addUserObjects() {}
  virtual void addCorrectors() {}
  virtual void addMultiApps() {}
  virtual void addTransfers() {}
  virtual void addPostprocessors() {}
  virtual void addVectorPostprocessors() {}
  virtual void addReporters() {}
  virtual void addOutputs() {}
  virtual void addPreconditioning() {}
  virtual void addExecutioner() {}
  virtual void addExecutors() {}

  /// Check the list of required tasks for missing tasks
  void checkRequiredTasks() const;

  /// Whether the physics is to be solved as a transient. It can be advantageous to solve
  /// some physics directly to steady state
  MooseEnum _is_transient;

  /// Vector of the solver variables (nonlinear and linear) in the Physics
  std::vector<VariableName> _solver_var_names;
  /// Vector of the aux variables in the Physics
  std::vector<VariableName> _aux_var_names;

  /// Dimension of the physics, which we expect for now to be the dimension of the mesh
  /// NOTE: this is not known at construction time, only after initializePhysics which is a huge bummer
  unsigned int _dim = libMesh::invalid_uint;

  /// Manually keeps track of the tasks required by each physics as tasks cannot be inherited
  std::set<std::string> _required_tasks;
};

template <typename T>
const T *
PhysicsBase::getCoupledPhysics(const PhysicsName & phys_name, const bool allow_fail) const
{
  constexpr bool is_physics = std::is_base_of<PhysicsBase, T>::value;
  libmesh_ignore(is_physics);
  mooseAssert(is_physics, "Must be a PhysicsBase to be retrieved by getCoupledPhysics");
  const auto all_T_physics = _awh.getActions<T>();
  for (const auto * const physics : all_T_physics)
  {
    if (physics->name() == phys_name)
      return physics;
  }
  if (!allow_fail)
    mooseError("Requested Physics '",
               phys_name,
               "' does not exist or is not of type '",
               MooseUtils::prettyCppType<T>(),
               "'");
  else
    return nullptr;
}

template <typename T>
const std::vector<T *>
PhysicsBase::getCoupledPhysics(const bool allow_fail) const
{
  constexpr bool is_physics = std::is_base_of<PhysicsBase, T>::value;
  libmesh_ignore(is_physics);
  mooseAssert(is_physics, "Must be a PhysicsBase to be retrieved by getCoupledPhysics");
  const auto all_T_physics = _awh.getActions<T>();
  if (!allow_fail && all_T_physics.empty())
    mooseError("No Physics of requested type '", MooseUtils::prettyCppType<T>(), "'");
  else
    return all_T_physics;
}

template <typename T>
void
PhysicsBase::checkComponentType(const ActionComponent & component) const
{
  if (!dynamic_cast<const T *>(&component))
    mooseError("Component '" + component.name() + "' must be of type '" +
               MooseUtils::prettyCppType<T>() + "'.\nIt is currently of type '" + component.type() +
               "'");
}
