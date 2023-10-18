//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralUserObject.h"

class Kernel;
class FVKernel;
class DGKernel;
class NodalKernel;
class DiracKernel;
class ScalarKernel;
class InterfaceKernel;
class FVInterfaceKernel;

/**
 * Base class to help creates an entire physics
 * Provides:
 * - containers and getters for MOOSE objects necessary for defining a Physics
 * - APIs to interact with Components
 * - utilities for passing and verifying parameters from the Physics block to MOOSE objects
 * - utilities for debugging
 */
class PhysicsBase : public GeneralUserObject
{
public:
  static InputParameters validParams();

  PhysicsBase(const InputParameters & parameters);

  /// A new blocks to the Physics
  void addBlocks(const std::vector<SubdomainName> & blocks);

  /// The default implementation of these routines will do nothing as we do not expect all Physics
  /// to be defining an object of every type
  virtual void addNonlinearVariables() {}
  virtual void addAuxiliaryVariables() {}
  virtual void addInitialConditions() {}
  virtual void addFEKernels() {}
  virtual void addFEBCs() {}
  virtual void addFVKernels() {}
  virtual void addFVBCs() {}
  virtual void addAuxiliaryKernels() {}
  virtual void addMaterials() {}
  virtual void addFunctorMaterials() {}
  virtual void addUserObjects() {}
  virtual void addPostprocessors() {}

protected:
  bool isTransient() const;

  // TODO : Remove virtual
  /// Get the factory for this physics
  /// The factory lets you get the parameters for objects
  virtual Factory & getFactory() { return *_factory; }
  /// Get the problem for this physics
  /// Useful to add objects to the simulation
  virtual FEProblemBase & getProblem() { return *_problem; }
  virtual FEProblemBase & getProblem() const { return *_problem; }
  /// Get the mesh for this physics
  /// This could be set by a component
  /// NOTE: hopefully we will not need this
  // virtual const MooseMesh & getMesh() const override { return *_mesh; }
  /// Tell the app if we want to use Exodus restart
  void prepareCopyNodalVariables() const;
  /// Copy variables from the mesh file
  void copyVariablesFromMesh(std::vector<VariableName> variables_to_copy);

  /// Utilities to check parameters
  /// These will be replaced by being baked into the validParams() logic, one day
  /// Check in debug mode that this parameter has been added to the validParams
  template <typename T>
  void assertParamDefined(const std::string & param1) const;
  /// Check that the two vector parameters are of the same length
  template <typename T, typename S>
  void checkVectorParamsSameLength(const std::string & param1, const std::string & param2) const;
  /// Check that this vector parameter (param1) has the same length as the MultiMooseEnum (param2)
  template <typename T>
  void checkVectorParamAndMultiMooseEnumLength(const std::string & param1,
                                               const std::string & param2) const;
  /// Check that the two vector of vector parameters are the same length
  template <typename T, typename S>
  void checkTwoDVectorParamsSameLength(const std::string & param1,
                                       const std::string & param2) const;
  template <typename T, typename S>
  void checkTwoDVectorParamInnerSameLengthAsOneDVector(const std::string & param1,
                                                       const std::string & param2) const;
  /// Check that there is no overlap between the two vector parameters
  template <typename T>
  void checkVectorParamsNoOverlap(const std::vector<std::string> & param_vec) const;
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

  /// Check that two parameters are either both set or both not set
  void checkParamsBothSetOrNotSet(const std::string & param1, const std::string & param2) const;
  void checkSecondParamSetOnlyIfFirstOneTrue(const std::string & param1,
                                             const std::string & param2) const;
  void checkSecondParamSetOnlyIfFirstOneSet(const std::string & param1,
                                            const std::string & param2) const;
  /// Check if the user commited errors during the definition of block-wise parameters
  template <typename T>
  void checkBlockwiseConsistency(const std::string & block_param_name,
                                 const std::vector<std::string> & parameter_names) const;
  /// Check that all shared parameters are consistent: if set (default or user), set to the same value
  void checkCommonParametersConsistent(const InputParameters & parameters) const;
  template <typename T>
  void warnInconsistent(const InputParameters & parameters, const std::string & param_name) const;
  /// Error messages for parameter checks
  void errorDependentParameter(const std::string & param1,
                               const std::string & value_not_set,
                               std::vector<std::string> dependent_params) const;

  /// Utilities to process and forward parameters
  void assignBlocks(InputParameters & params, const std::vector<SubdomainName> & blocks) const;
  /// Checks if the variables created outside of the physics are restricted to the same blocks
  // void checkVariableBlockRestrictionConsistency(const std::string & var_name);
  // USE CHECKVARIABLE FROM BLOCKRESITRCATBLE

  /// Use prefix() to disambiguate names
  std::string prefix() const { return name() + "_"; }

  /// TODO: interaction with components
  void processMesh(){};

  /// Return the list of nonlinear variables in this physics
  std::vector<VariableName> nonlinearVariableNames() const { return _nl_var_names; };
  /// Keep track of the name of a nonlinear variable defined in the Physics
  void saveNonlinearVariableName(const VariableName & var_name)
  {
    _nl_var_names.push_back(var_name);
  }

  /// Whether to output additional information
  const bool _verbose;

  /// Dimension of the physics, which we expect for now to be the dimension of the mesh
  unsigned int _dim;

  // The block restrictable interface is not adapted to keeping track of a growing list of blocks as
  // we add more parameters
  std::vector<SubdomainName> _blocks;

private:
  /// Add any relationship manager needed by the physics, for example for 'ghosting' layers of
  /// near process boundaries
  // virtual void addRelationshipManagers(Moose::RelationshipManagerType input_rm_type);

  /// Whether the physics is to be solved as a transient. It can be advantageous to solve
  /// some physics directly to steady state
  MooseEnum _is_transient;

  /// Pointer to the Factory associated with the MooseApp
  Factory * _factory;
  /// Pointer to the problem this physics works with
  FEProblemBase * _problem;

  /// Vector of the nonlinear variables in the Physics
  std::vector<VariableName> _nl_var_names;

  /// Needed to create every object
  friend class AddPhysicsAction;
  friend class FileMeshPhysicsComponent;
  friend class FileMeshFlowComponent;
};

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

template <typename T>
void
PhysicsBase::assertParamDefined(const std::string & libmesh_dbg_var(param1)) const
{
  mooseAssert(parameters().have_parameter<T>(param1),
              "Parameter '" + param1 + "' is not defined with type " +
                  MooseUtils::prettyCppType<T>() + ". Check your code.");
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
        mooseError("Item " + value + "specified in vector parameter " + param +
                   " is also present in one or more of these other parameters " +
                   Moose::stringify(param_vec) + ". This is disallowed");
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
void
PhysicsBase::warnInconsistent(const InputParameters & other_param,
                              const std::string & param_name) const
{
  assertParamDefined<T>(param_name);
  mooseAssert(other_param.have_parameter<T>(param_name),
              "This should have been a parameter from the parameters being compared");
  bool warn = false;
  if (parameters().isParamValid(param_name) && other_param.isParamValid(param_name))
  {
    if constexpr (std::is_same_v<MooseEnum, T>)
    {
      if (!getParam<T>(param_name).compareCurrent(other_param.get<T>(param_name)))
        warn = true;
    }
    else if (getParam<T>(param_name) != other_param.get<T>(param_name))
      warn = true;
  }
  if (warn)
    mooseWarning("Parameter " + param_name +
                 " is inconsistent between this physics and the parameter set for \"" +
                 other_param.get<std::string>("_object_name") + "\" of type \"" +
                 other_param.get<std::string>("_type") + "\"");
}
