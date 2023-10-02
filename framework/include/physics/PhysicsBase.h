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
  bool isTransient() const { return _is_transient; }

  /// Get the factory for this physics
  /// The factory lets you get the parameters for objects
  virtual Factory & getFactory() { return *_factory; }
  /// Get the problem for this physics
  /// Useful to add objects to the simulation
  virtual FEProblemBase & getProblem() { return *_problem; }
  /// Get the mesh for this physics
  /// This could be set by a component
  /// NOTE: hopefully we will not need this
  // virtual const MooseMesh & getMesh() const override { return *_mesh; }

private:
  virtual void addFEKernels(){};
  virtual void addNonlinearVariables(){};

  /// Whether the physics is to be solved as a transient. It can be advantageous to solve
  /// some physics directly to steady state
  bool _is_transient;

  /// The Factory associated with the MooseApp
  Factory * _factory;
  /// Convenience reference to a problem this action works on
  std::shared_ptr<FEProblemBase> _problem;

  /// Needed to create every object
  friend class AddPhysicsAction;
};
