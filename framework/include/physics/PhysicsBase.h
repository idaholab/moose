//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Action.h"

// We include these headers for all the derived classes that will be building objects
#include "FEProblemBase.h"
#include "Factory.h"

#define registerPhysicsBaseTasks(app_name, derived_name)                                           \
  registerMooseAction(app_name, derived_name, "init_physics")

/**
 * Base class to help creates an entire physics
 */
class PhysicsBase : public Action
{
public:
  static InputParameters validParams();

  PhysicsBase(const InputParameters & parameters);

  /// Forwards from the action tasks to the implemented addXYZ() in the derived classes
  /// If you need more than these:
  /// - register your action to the new task using
  ///   registerMooseAction("AppName", ActionClass, "task_name");
  /// - override act and add your additional work there
  virtual void act() override final;

  /// Add new blocks to the Physics
  void addBlocks(const std::vector<SubdomainName> & blocks);

  /// Provide additional parameters for the relationship managers
  virtual InputParameters getAdditionalRMParams() const { return emptyInputParameters(); };

  /// Get a Physics from the ActionWarehouse with the requested type and name
  template <typename T>
  const T * getCoupledPhysics(const PhysicsName & phys_name) const;

protected:
  /// Return whether the Physics is solved using a transient
  bool isTransient() const;
  /// Return the maximum dimension of the blocks the Physics is active on
  unsigned int dimension() const;

  /// Get the factory for this physics
  /// The factory lets you get the parameters for objects
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
  void prepareCopyNodalVariables() const;
  /// Copy variables from the mesh file
  void copyVariablesFromMesh(const std::vector<VariableName> & variables_to_copy);

  /// Use prefix() to disambiguate names
  std::string prefix() const { return name() + "_"; }

  /// Return the list of nonlinear variables in this physics
  const std::vector<VariableName> & nonlinearVariableNames() const { return _nl_var_names; };
  /// Keep track of the name of a nonlinear variable defined in the Physics
  void saveNonlinearVariableName(const VariableName & var_name)
  {
    _nl_var_names.push_back(var_name);
  }

  // BEGIN: Utilities for checking parameters.
  // These will be replaced by being baked into the validParams() logic, one day
  /// Check in debug mode that this parameter has been added to the validParams
  template <typename T>
  void assertParamDefined(const std::string & param) const;
  /// Check that two parameters are either both set or both not set
  void checkParamsBothSetOrNotSet(const std::string & param1, const std::string & param2) const;
  /// Check that a parameter is set only if the first one is set to true
  void checkSecondParamSetOnlyIfFirstOneTrue(const std::string & param1,
                                             const std::string & param2) const;
  /// Check that the two vector parameters are of the same length
  template <typename T, typename S>
  void checkVectorParamsSameLength(const std::string & param1, const std::string & param2) const;
  /// Check that there is no overlap between the items in each vector parameters
  /// Each vector parameter should also have unique items
  template <typename T>
  void checkVectorParamsNoOverlap(const std::vector<std::string> & param_vecs) const;

  // END: parameter checking utilities

  /// Check whether a nonlinear variable already exists
  bool nonlinearVariableExists(const VariableName & var_name, bool error_if_aux) const;

  /// Add a new required task for all physics deriving from this class
  /// NOTE: This does not register the task, you still need to call registerMooseAction
  void addRequiredPhysicsTask(const std::string & task) { _required_tasks.insert(task); }

  /// System number for the system owning the variables
  const unsigned int _sys_number;

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

  /// The default implementation of these routines will do nothing as we do not expect all Physics
  /// to be defining an object of every type
  virtual void addNonlinearVariables() {}
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

  /// Vector of the nonlinear variables in the Physics
  std::vector<VariableName> _nl_var_names;

  /// Dimension of the physics, which we expect for now to be the dimension of the mesh
  /// NOTE: this is not known at construction time, only after initializePhysics which is a huge bummer
  unsigned int _dim = libMesh::invalid_uint;

  /// Manually keeps track of the tasks required by each physics as tasks cannot be inherited
  std::set<std::string> _required_tasks;
};

template <typename T>
const T *
PhysicsBase::getCoupledPhysics(const PhysicsName & phys_name) const
{
  const auto all_T_physics = _awh.getActions<T>();
  for (const auto * const physics : all_T_physics)
  {
    if (physics->name() == phys_name)
      return physics;
  }
  mooseError("Requested Physics '",
             phys_name,
             "' does not exist or is not of type '",
             MooseUtils::prettyCppType<T>(),
             "'");
}

template <typename T>
void
PhysicsBase::assertParamDefined(const std::string & libmesh_dbg_var(param)) const
{
  mooseAssert(parameters().have_parameter<T>(param),
              "Parameter '" + param + "' is not defined with type '" +
                  MooseUtils::prettyCppType<T>() + "' in object type '" +
                  MooseUtils::prettyCppType(type()) + "'. Check your code.");
}

template <typename T, typename S>
void
PhysicsBase::checkVectorParamsSameLength(const std::string & param1,
                                         const std::string & param2) const
{
  assertParamDefined<std::vector<T>>(param1);
  assertParamDefined<std::vector<S>>(param2);

  if (isParamValid(param1) && isParamValid(param2))
  {
    const auto size_1 = getParam<std::vector<T>>(param1).size();
    const auto size_2 = getParam<std::vector<S>>(param2).size();
    if (size_1 != size_2)
      paramError(param1,
                 "Vector parameters '" + param1 + "' (size " + std::to_string(size_1) + ") and '" +
                     param2 + "' (size " + std::to_string(size_2) + ") must be the same size");
  }
  // handle empty vector defaults
  else if (isParamValid(param1) || isParamValid(param2))
    if (getParam<std::vector<T>>(param1).size() || getParam<std::vector<T>>(param2).size())
      checkParamsBothSetOrNotSet(param1, param2);
}

template <typename T>
void
PhysicsBase::checkVectorParamsNoOverlap(const std::vector<std::string> & param_vec) const
{
  std::set<std::string> unique_params;
  for (const auto & param : param_vec)
  {
    assertParamDefined<std::vector<T>>(param);

    for (const auto & value : getParam<std::vector<T>>(param))
      if (!unique_params.insert(value).second)
        mooseError("Item '" + value + "' specified in vector parameter '" + param +
                   "' is also present in one or more of the parameters '" +
                   Moose::stringify(param_vec) + "'. This is disallowed.");
  }
}
