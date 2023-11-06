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
  void act() override;

  /// Add a new blocks to the Physics
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
  Factory & getFactory() { return _factory; }
  Factory & getFactory() const { return _factory; }
  /// Get the problem for this physics
  /// Useful to add objects to the simulation
  virtual FEProblemBase & getProblem()
  {
    mooseAssert(_problem, "Requesting the problem too early");
    return *_problem;
  }
  virtual FEProblemBase & getProblem() const
  {
    mooseAssert(_problem, "Requesting the problem too early");
    return *_problem;
  }

  /// Tell the app if we want to use Exodus restart
  void prepareCopyNodalVariables() const;
  /// Copy variables from the mesh file
  void copyVariablesFromMesh(std::vector<VariableName> variables_to_copy);

  /// Use prefix() to disambiguate names
  std::string prefix() const { return name() + "_"; }

  /// Return the list of nonlinear variables in this physics
  std::vector<VariableName> nonlinearVariableNames() const { return _nl_var_names; };
  /// Keep track of the name of a nonlinear variable defined in the Physics
  void saveNonlinearVariableName(const VariableName & var_name)
  {
    _nl_var_names.push_back(var_name);
  }

  /// Whether to output additional information
  const bool _verbose;

  /// Keep track of the subdomains the Physics is defined on
  std::vector<SubdomainName> _blocks;

private:
  /// Gathers additional parameters for the relationship managers from the Physics
  /// then calls the parent Action::addRelationshipManagers with those parameters
  using Action::addRelationshipManagers;
  void addRelationshipManagers(Moose::RelationshipManagerType input_rm_type) override;

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

  /// Whether the physics is to be solved as a transient. It can be advantageous to solve
  /// some physics directly to steady state
  MooseEnum _is_transient;

  /// Vector of the nonlinear variables in the Physics
  std::vector<VariableName> _nl_var_names;

  /// Dimension of the physics, which we expect for now to be the dimension of the mesh
  /// NOTE: this is not known at construction time, which is a huge bummer
  unsigned int _dim;
};

template <typename T>
const T *
PhysicsBase::getCoupledPhysics(const PhysicsName & phys_name) const
{
  auto all_T_physics = _awh.getActions<T>();
  for (auto physics : all_T_physics)
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
