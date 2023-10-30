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
#include "MultiMooseEnum.h"

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
  /// - override actOnAdditionalTasks and add your additional work there
  virtual void act() override final;

  /// Routine to add additional setup work on additional registered tasks to a Physics
  virtual void actOnAdditionalTasks() {}

  /// Add new blocks to the Physics
  void addBlocks(const std::vector<SubdomainName> & blocks);

  /// Provide additional parameters for the relationship managers
  virtual InputParameters getAdditionalRMParams() const { return emptyInputParameters(); };

  /// Get a Physics from the ActionWarehouse with the requested type and name
  template <typename T>
  const T * getCoupledPhysics(const PhysicsName & phys_name) const;

  /// Utilities to merge two Physics of the same type together
  /// Check that parameters are compatible for a merge with another Physics
  virtual bool checkParametersMergeable(const InputParameters & /*param*/, bool /*warn*/) const
  {
    mooseError("Not implemented");
  }
  /// Merge these parameters into existing parameters of this Physics
  virtual void mergeParameters(const InputParameters & /*params*/)
  {
    mooseError("Not implemented");
  }

protected:
  /// Return whether the Physics is solved using a transient
  bool isTransient() const;
  /// Return the maximum dimension of the blocks the Physics is active on
  unsigned int dimension() const;

  /// Get the factory for this physics
  /// The factory lets you get the parameters for objects
  virtual Factory & getFactory() { return _factory; }
  virtual Factory & getFactory() const { return _factory; }
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

  /// Get the mesh for this physics
  /// This could be set by a component
  /// NOTE: hopefully we will not need this
  // virtual const MooseMesh & getMesh() const override { return *_mesh; }
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
  void checkSecondParamSetOnlyIfFirstOneSet(const std::string & param1,
                                            const std::string & param2) const;
  /// Check that the two vector parameters are of the same length
  template <typename T, typename S>
  void checkVectorParamsSameLength(const std::string & param1, const std::string & param2) const;
  /// Check that this vector parameter (param1) has the same length as the MultiMooseEnum (param2)
  template <typename T>
  void checkVectorParamAndMultiMooseEnumLength(const std::string & param1,
                                               const std::string & param2) const;
  template <typename T, typename S>
  void checkTwoDVectorParamsSameLength(const std::string & param1,
                                       const std::string & param2) const;
  /// Check that there is no overlap between the items in each vector parameters
  /// Each vector parameter should also have unique items
  template <typename T>
  void checkVectorParamsNoOverlap(const std::vector<std::string> & param_vecs) const;
  template <typename T, typename S>
  void checkTwoDVectorParamInnerSameLengthAsOneDVector(const std::string & param1,
                                                       const std::string & param2) const;
  bool nonLinearVariableExists(const VariableName & var_name, bool error_if_aux) const;
  /// Check that the user did not pass an empty vector
  template <typename T>
  void checkVectorParamNotEmpty(const std::string & param1) const;
  /// Check that two vector parameters are the same length if both are set
  template <typename T, typename S>
  void checkVectorParamsSameLengthIfSet(const std::string & param1,
                                        const std::string & param2) const;

  template <typename T, typename S, typename U>
  void checkVectorParamLengthSameAsCombinedOthers(const std::string & param1,
                                                  const std::string & param2,
                                                  const std::string & param3) const;

  /// Check if the user commited errors during the definition of block-wise parameters
  template <typename T>
  void checkBlockwiseConsistency(const std::string & block_param_name,
                                 const std::vector<std::string> & parameter_names) const;
  /// Check if an external object has the same block restriction
  void checkBlockRestrictionIdentical(const std::string & object_name,
                                      const std::vector<SubdomainName> & blocks) const;
  /// Check that all shared parameters are consistent: if set (default or user), set to the same value
  void checkCommonParametersConsistent(const InputParameters & parameters) const;
  template <typename T>
  bool parameterConsistent(const InputParameters & other_param,
                           const std::string & param_name,
                           bool warn) const;
  template <typename T>
  void warnInconsistent(const InputParameters & parameters, const std::string & param_name) const;
  /// Error messages for parameter checks
  void errorDependentParameter(const std::string & param1,
                               const std::string & value_not_set,
                               const std::vector<std::string> & dependent_params) const;
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

  /// Utilities to process and forward parameters
  void assignBlocks(InputParameters & params, const std::vector<SubdomainName> & blocks) const;
  /// Checks if the variables created outside of the physics are restricted to the same blocks
  // void checkVariableBlockRestrictionConsistency(const std::string & var_name);
  // USE CHECKVARIABLE FROM BLOCKRESITRCATBLE

  /// Routine to help create maps
  template <typename T, typename C>
  std::map<T, C> createMapFromVectors(std::vector<T> keys, std::vector<C> values) const;
  template <typename T>
  std::map<T, MooseEnum> createMapFromVectorAndMultiMooseEnum(std::vector<T> keys,
                                                              MultiMooseEnum values) const;

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
PhysicsBase::checkVectorParamAndMultiMooseEnumLength(const std::string & param1,
                                                     const std::string & param2) const
{
  assertParamDefined<std::vector<T>>(param1);
  assertParamDefined<MultiMooseEnum>(param2);

  if (isParamValid(param1) && isParamValid(param2))
  {
    const auto size_1 = getParam<std::vector<T>>(param1).size();
    const auto size_2 = getParam<MultiMooseEnum>(param2).size();
    if (size_1 != size_2)
      paramError(param1,
                 "Vector parameters '" + param1 + "' (size " + std::to_string(size_1) + ") and '" +
                     param2 + "' (size " + std::to_string(size_2) + ") must be the same size");
  }
  // handle empty vector defaults
  else if (isParamValid(param1) || isParamValid(param2))
    if (getParam<std::vector<T>>(param1).size() || getParam<MultiMooseEnum>(param2).size())
      checkParamsBothSetOrNotSet(param1, param2);
}

template <typename T, typename S>
void
PhysicsBase::checkTwoDVectorParamsSameLength(const std::string & param1,
                                             const std::string & param2) const
{
  checkVectorParamsSameLength<std::vector<T>, std::vector<S>>(param1, param2);
  if (isParamValid(param1) && isParamValid(param2))
  {
    const auto value1 = getParam<std::vector<std::vector<T>>>(param1);
    const auto value2 = getParam<std::vector<std::vector<S>>>(param2);
    for (const auto index : index_range(value1))
      if (value1[index].size() != value2[index].size())
        paramError(param1,
                   "Vector at index " + std::to_string(index) + " of 2D vector parameter '" +
                       param1 +
                       "' is not the same size as its counterpart from 2D vector parameter '" +
                       param2 + "'");
  }
  // handle empty vector defaults
  else if (isParamValid(param1) || isParamValid(param2))
    if (getParam<std::vector<T>>(param1).size() || getParam<std::vector<T>>(param2).size())
      checkParamsBothSetOrNotSet(param1, param2);
}

template <typename T, typename S>
void
PhysicsBase::checkTwoDVectorParamInnerSameLengthAsOneDVector(const std::string & param1,
                                                             const std::string & param2) const
{
  assertParamDefined<std::vector<std::vector<T>>>(param1);
  assertParamDefined<std::vector<S>>(param2);
  for (const auto & sub_vec_i : index_range(getParam<std::vector<std::vector<T>>>(param1)))
  {
    const auto size_1 = getParam<std::vector<std::vector<T>>>(param1)[sub_vec_i].size();
    const auto size_2 = getParam<std::vector<S>>(param2).size();
    if (size_1 != size_2)
      paramError(param1,
                 "Vector at index " + std::to_string(sub_vec_i) + " (size " +
                     std::to_string(size_1) +
                     ") "
                     " of this parameter should be the same length as parameter '" +
                     param2 + "' (size " + std::to_string(size_2) + ")");
  }
}

template <typename T, typename S, typename U>
void
PhysicsBase::checkVectorParamLengthSameAsCombinedOthers(const std::string & param1,
                                                        const std::string & param2,
                                                        const std::string & param3) const
{
  assertParamDefined<std::vector<T>>(param1);
  assertParamDefined<std::vector<S>>(param2);
  assertParamDefined<std::vector<U>>(param3);
  const auto size_1 = getParam<std::vector<T>>(param1).size();
  const auto size_2 = getParam<std::vector<S>>(param2).size();
  const auto size_3 = getParam<std::vector<U>>(param3).size();

  if (size_1 != size_2 + size_3)
    paramError(param1,
               "Vector parameter '" + param1 + "' (size " + std::to_string(size_1) +
                   ") should be the same size as parameter '" + param2 + "' and '" + param3 +
                   " combined (total size " + std::to_string(size_2 + size_3) + ")");
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

template <typename T>
void
PhysicsBase::checkVectorParamNotEmpty(const std::string & param) const
{
  assertParamDefined<std::vector<T>>(param);
  if (!getParam<std::vector<T>>(param).size())
    paramError(param, "Parameter '" + param + "' should not be set to an empty vector.");
}

template <typename T, typename S>
void
PhysicsBase::checkVectorParamsSameLengthIfSet(const std::string & param1,
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
                 "Parameter '" + param1 + "' (size " + std::to_string(size_1) + ") and '" + param2 +
                     "' (size " + std::to_string(size_2) + ") must be the same size if set.");
  }
}

template <typename T>
void
PhysicsBase::checkBlockwiseConsistency(const std::string & block_param_name,
                                       const std::vector<std::string> & parameter_names) const
{
  const std::vector<std::vector<SubdomainName>> & block_names =
      getParam<std::vector<std::vector<SubdomainName>>>(block_param_name);

  if (block_names.size())
  {
    // We only check block-restrictions if the action is not restricted to `ANY_BLOCK_ID`.
    // If the users define blocks that are not on the mesh, they will receive errors from the
    // objects created created by the action
    if (std::find(_blocks.begin(), _blocks.end(), "ANY_BLOCK_ID") == _blocks.end())
      for (const auto & block_group : block_names)
        for (const auto & block : block_group)
          if (std::find(_blocks.begin(), _blocks.end(), block) == _blocks.end())
            paramError(block_param_name,
                       "Block '" + block +
                           "' is not present in the block restriction of the fluid flow action!");

    for (const auto & param_name : parameter_names)
    {
      const std::vector<T> & param_vector = getParam<std::vector<T>>(param_name);
      if (block_names.size() != param_vector.size())
        paramError(param_name,
                   "The number of entries in '" + param_name + "' (" +
                       std::to_string(param_vector.size()) +
                       ") is not the same as the number of blocks"
                       " (" +
                       std::to_string(block_names.size()) + ") in '" + block_param_name + "'!");
    }
  }
  else
  {
    unsigned int previous_size = 0;
    for (unsigned int param_i = 0; param_i < parameter_names.size(); ++param_i)
    {
      const std::vector<T> & param_vector = getParam<std::vector<T>>(parameter_names[param_i]);
      if (param_i == 0)
      {
        if (param_vector.size() > 1)
          paramError(parameter_names[param_i],
                     "The user should only use one or zero entries in " + parameter_names[param_i] +
                         " if " + block_param_name + " not defined!");
        previous_size = param_vector.size();
      }
      else
      {
        if (previous_size != param_vector.size())
          paramError(parameter_names[param_i],
                     "The number of entries in '" + parameter_names[param_i] +
                         "' is not the same as the number of entries in '" +
                         parameter_names[param_i - 1] + "'!");
      }
    }
  }
}

template <typename T>
bool
PhysicsBase::parameterConsistent(const InputParameters & other_param,
                                 const std::string & param_name,
                                 bool warn) const
{
  assertParamDefined<T>(param_name);
  mooseAssert(other_param.have_parameter<T>(param_name),
              "This should have been a parameter from the parameters being compared");
  bool consistent = true;
  if (parameters().isParamValid(param_name) && other_param.isParamValid(param_name))
  {
    if constexpr (std::is_same_v<MooseEnum, T>)
    {
      if (!getParam<T>(param_name).compareCurrent(other_param.get<T>(param_name)))
        consistent = false;
    }
    else if (getParam<T>(param_name) != other_param.get<T>(param_name))
      consistent = false;
  }
  if (warn && !consistent)
    mooseWarning("Parameter " + param_name + " is inconsistent between Physics \"" + name() +
                 "\" of type \"" + type() + "\" and the parameter set for \"" +
                 other_param.get<std::string>("_action_name") + "\" of type \"" +
                 other_param.get<std::string>("action_type") + "\"");
  return consistent;
}

template <typename T>
void
PhysicsBase::warnInconsistent(const InputParameters & other_param,
                              const std::string & param_name) const
{
  parameterConsistent<T>(other_param, param_name, true);
}

template <typename T, typename C>
std::map<T, C>
PhysicsBase::createMapFromVectors(std::vector<T> keys, std::vector<C> values) const
{
  std::map<T, C> map;
  // No values have been specified.
  if (!values.size())
  {
    // If we cant return a map of default C, dont try it
    if constexpr (std::is_same_v<MooseEnum, T> || std::is_same_v<MultiMooseEnum, T>)
      return map;

    C def;
    for (const auto & k : keys)
      map[k] = def;
    return map;
  }
  std::transform(keys.begin(),
                 keys.end(),
                 values.begin(),
                 std::inserter(map, map.end()),
                 [](T a, C b) { return std::make_pair(a, b); });
  return map;
}

template <typename T>
std::map<T, MooseEnum>
PhysicsBase::createMapFromVectorAndMultiMooseEnum(std::vector<T> keys, MultiMooseEnum values) const
{
  std::map<T, MooseEnum> map;
  // No values have been specified. We cant form a map of empty MooseEnum
  if (!values.size())
    return map;
  std::transform(keys.begin(),
                 keys.end(),
                 values.begin(),
                 std::inserter(map, map.end()),
                 [values](T a, MooseEnumItem b)
                 {
                   // Create a MooseEnum from the available values in the MultiMooseEnum and an
                   // actual current active item from that same MultiMooseEnum
                   MooseEnum single_value(values.getRawNames(), b.name());
                   return std::make_pair(a, single_value);
                 });
  return map;
}
