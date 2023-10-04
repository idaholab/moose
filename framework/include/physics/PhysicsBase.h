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
#include "PhysicsDiscretization.h"

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
  /// Get the discretization object
  virtual PhysicsDiscretization & getDiscretization() { return *_discretization; }

  /// Utilities to handle parameters
  void checkParamsBothSetOrNotSet(std::string param1, std::string param2) const;
  template <typename T, typename S>
  void checkVectorParamsSameLength(std::string param1, std::string param2) const;
  template <typename T>
  void checkVectorParamsNoOverlap(std::vector<std::string> param_vec) const;

  // TODO once design validated, make private, use a setter
  /// Pointer to the discretized physics object
  std::unique_ptr<PhysicsBase> _discretized_physics;

private:
  /// The default implementation of these routines will do nothing as we do not expect all Physics
  /// to be defining an object of every type
  /// We keep these private for now, may become public soon
  virtual void addNonlinearVariables()
  {
    mooseAssert(_discretized_physics, "Should have been created");
    _discretized_physics->addNonlinearVariables();
  }
  virtual void addFEKernels() { _discretized_physics->addFEKernels(); }
  virtual void addFEBCs() { _discretized_physics->addFEBCs(); }
  virtual void addDiscretization(const InputParameters & params);
  virtual void createDiscretizedPhysics() = 0;

  /// Whether the physics is to be solved as a transient. It can be advantageous to solve
  /// some physics directly to steady state
  MooseEnum _is_transient;

  /// Pointer to the Factory associated with the MooseApp
  Factory * _factory;
  /// Pointer to the problem this physics works with
  FEProblemBase * _problem;
  /// Point to the discretization object
  std::shared_ptr<PhysicsDiscretization> _discretization;

  /// Needed to create every object
  friend class AddPhysicsAction;
  friend class AddPhysicsDiscretizationAction;
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
    if (getParam<std::vector<T>>(param1).size() != getParam<std::vector<S>>(param2).size())
      paramError(param1,
                 "Vector parameters " + param1 + " and " + param2 +
                     " must be the same length if set");
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
