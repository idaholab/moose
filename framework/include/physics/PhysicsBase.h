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
#include "BlockRestrictable.h"

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
class PhysicsBase : public GeneralUserObject, public BlockRestrictable
{
public:
  static InputParameters validParams();

  PhysicsBase(const InputParameters & parameters);

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
  /// Check that there is no overlap between the two vector parameters
  template <typename T>
  void checkVectorParamsNoOverlap(const std::vector<std::string> & param_vec) const;
  bool nonLinearVariableExists(const VariableName & var_name, bool error_if_aux) const;
  /// Check that the dependent parameters all are (or are not) defined when the main parameter is
  void checkDependentParameterError(const std::string & main_parameter,
                                    const std::vector<std::string> & dependent_parameters,
                                    const bool should_be_defined) const;
  /// Check that two parameters are either both set or both not set
  void checkParamsBothSetOrNotSet(const std::string & param1, const std::string & param2) const;
  void checkSecondParamSetOnlyIfFirstOneTrue(const std::string & param1,
                                             const std::string & param2) const;
  /// Check if the user commited errors during the definition of block-wise parameters
  template <typename T>
  void checkBlockwiseConsistency(const std::string & block_param_name,
                                 const std::vector<std::string> parameter_names) const;

  /// Utilities to process and forward parameters
  void assignBlocks(InputParameters & params, const std::vector<SubdomainName> & blocks) const;
  /// Checks if the variables created outside of the physics are restricted to the same blocks
  // void checkVariableBlockRestrictionConsistency(const std::string & var_name);
  // USE CHECKVARIABLE FROM BLOCKRESITRCATBLE

  /// Use prefix() to disambiguate names
  std::string prefix() const { return name() + "_"; }

  /// TODO: interaction with components
  void processMesh(){};

  /// Dimension of the physics, which we expect for now to be the dimension of the mesh
  unsigned int _dim;

  /// TODO: see if we can rely on BlockRestrictable instead
  std::vector<SubdomainName> _blocks;

private:
  /// The default implementation of these routines will do nothing as we do not expect all Physics
  /// to be defining an object of every type
  /// We keep these private for now, may become public soon
  virtual void addNonlinearVariables() {}
  virtual void addInitialConditions(){};
  virtual void addFEKernels() {}
  virtual void addFEBCs() {}
  virtual void addFVKernels() {}
  virtual void addFVBCs() {}
  virtual void addMaterials() {}
  virtual void addFunctorMaterials() {}
  virtual void addUserObjects() {}
  virtual void addPostprocessors() {}

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

  /// Needed to create every object
  friend class AddPhysicsAction;
};

template <typename T, typename S>
void
PhysicsBase::checkVectorParamsSameLength(const std::string & param1,
                                         const std::string & param2) const
{
  checkParamsBothSetOrNotSet(param1, param2);
  assertParamDefined<std::vector<T>>(param1);
  assertParamDefined<std::vector<S>>(param2);

  if (isParamValid(param1))
  {
    const auto size_1 = getParam<std::vector<T>>(param1).size();
    const auto size_2 = getParam<std::vector<S>>(param2).size();
    if (size_1 != size_2)
      paramError(param1,
                 "Vector parameters '" + param1 + "' (size " + std::to_string(size_1) + ") and '" +
                     param2 + "' (size " + std::to_string(size_2) + ") must be the same size");
  }
}

template <typename T>
void
PhysicsBase::checkVectorParamAndMultiMooseEnumLength(const std::string & param1,
                                                     const std::string & param2) const
{
  checkParamsBothSetOrNotSet(param1, param2);
  assertParamDefined<std::vector<T>>(param1);
  assertParamDefined<MultiMooseEnum>(param2);

  if (isParamValid(param1))
  {
    const auto size_1 = getParam<std::vector<T>>(param1).size();
    const auto size_2 = getParam<MultiMooseEnum>(param2).size();
    if (size_1 != size_2)
      paramError(param1,
                 "Vector parameters '" + param1 + "' (size " + std::to_string(size_1) + ") and '" +
                     param2 + "' (size " + std::to_string(size_2) + ") must be the same size");
  }
}

template <typename T>
void
PhysicsBase::assertParamDefined(const std::string & param1) const
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
  if (isParamValid(param1))
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
}

template <typename T>
void
PhysicsBase::checkVectorParamsNoOverlap(const std::vector<std::string> & param_vec) const
{
  std::set<std::string> unique_params;
  for (const auto & param : param_vec)

    for (const auto & value : getParam<std::vector<T>>(param))
      if (!unique_params.insert(value).second)
        mooseError("Item " + value + "specified in vector parameter " + param +
                   " is also present in one or more of these other parameters " +
                   Moose::stringify(param_vec) + ". This is disallowed");
}
