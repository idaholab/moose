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
  void checkParamsBothSetOrNotSet(std::string param1, std::string param2) const;
  template <typename T, typename S>
  void checkVectorParamsSameLength(std::string param1, std::string param2) const;
  template <typename T, typename S>
  void checkTwoDVectorParamsSameLength(std::string param1, std::string param2) const;
  template <typename T>
  void checkVectorParamsNoOverlap(std::vector<std::string> param_vec) const;
  bool nonLinearVariableExists(const VariableName & var_name, bool error_if_aux) const;
  void checkDependentParameterError(const std::string & main_parameter,
                                    const std::vector<std::string> & dependent_parameters,
                                    const bool should_be_defined) const;

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

  virtual void addRelationshipManagers(Moose::RelationshipManagerType /*input_rm_type*/) {}

  /// Assign the necessary blocks to the input parameters
  void assignBlocks(InputParameters & params, const std::vector<SubdomainName> & blocks) const;
  /// Checks if the variables created outside of the physics are restricted to the same blocks
  // void checkVariableBlockRestrictionConsistency(const std::string & var_name);
  // USE CHECKVARIABLE FROM BLOCKRESITRCATBLE

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
PhysicsBase::checkVectorParamsSameLength(std::string param1, std::string param2) const
{
  checkParamsBothSetOrNotSet(param1, param2);
  if (!parameters().have_parameter<std::vector<T>>(param1))
    paramError("Parameter ",
               param1,
               " is not defined with type ",
               MooseUtils::prettyCppType<T>(),
               ". Check your code.");
  if (!parameters().have_parameter<std::vector<S>>(param2))
    paramError("Parameter ",
               param2,
               " is not defined with type ",
               MooseUtils::prettyCppType<S>(),
               ". Check your code.");
  if (isParamValid(param1))
  {
    const auto size_1 = getParam<std::vector<T>>(param1).size();
    const auto size_2 = getParam<std::vector<S>>(param2).size();
    if (size_1 != size_2)
      paramError(param1,
                 "Vector parameters " + param1 + "(size " + std::to_string(size_1) + ") and " +
                     param2 + "(size " + std::to_string(size_2) + ") must be the same size");
  }
}

template <typename T, typename S>
void
PhysicsBase::checkTwoDVectorParamsSameLength(std::string param1, std::string param2) const
{
  checkVectorParamsSameLength<std::vector<T>, std::vector<S>>(param1, param2);
  if (isParamValid(param1))
  {
    const auto value1 = getParam<std::vector<std::vector<T>>>(param1);
    const auto value2 = getParam<std::vector<std::vector<S>>>(param2);
    for (const auto index : index_range(value1))
      if (value1[index].size() != value2[index].size())
        paramError(
            param1,
            "Vector at index " + std::to_string(index) + " of 2D vector parameter " + param1 +
                " is not the same size as its counterpart from 2D vector parameter " + param2);
  }
}

template <typename T>
void
PhysicsBase::checkVectorParamsNoOverlap(std::vector<std::string> param_vec) const
{
  std::set<std::string> unique_params;
  for (const auto & param : param_vec)

    for (const auto & value : getParam<std::vector<T>>(param))
      if (!unique_params.insert(value).second)
        mooseError("Item " + value + "specified in vector parameter " + param +
                   " is also present in one or more of these other parameters " +
                   Moose::stringify(param_vec) + ". This is disallowed");
}
