//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "Action.h"
#include "ActionWarehouse.h"
#include "InputParametersChecksUtils.h"

class PhysicsBase;
class FEProblemBase;

#define registerActionComponent(app_name, component_name)                                          \
  registerMooseAction(app_name, component_name, "list_component")

/**
 * Base class for components that are defined using an action
 */
class ActionComponent : public Action, public InputParametersChecksUtils<ActionComponent>
{
public:
  static InputParameters validParams();

  ActionComponent(const InputParameters & params);

  virtual void act() override final;

  /// Get the name(s) of the mesh generator(s) created by this component that generates the mesh for it
  /// - this could be a mesh generator in the [Mesh] block
  /// - or a mesh generator created by the component
  const std::vector<MeshGeneratorName> & meshGeneratorNames() const { return _mg_names; }

  /// Returns the subdomains for the component mesh, if any
  const std::vector<SubdomainName> & blocks() const { return _blocks; }

  /// Return the outer surface boundaries
  virtual const std::vector<BoundaryName> & outerSurfaceBoundaries() const
  {
    mooseError("Not implemented");
  };

  /// Return the component volume
  virtual Real volume() const { mooseError("Volume routine is not implemented"); }

  /// Return the component outer boundary area
  virtual Real outerSurfaceArea() const { mooseError("Outer surface area is not implemented"); }

  /// Return the dimension of the component
  unsigned int dimension() const { return _dimension; }

  /// Return mesh generator names of the component
  std::vector<MeshGeneratorName> mg_names() const { return _mg_names; }

protected:
  // The default implementation of these routines will do nothing as we do not expect all Components
  // to be defining an object of every type
  // These routines are to help define a strictly geometrical component
  virtual void addMeshGenerators() {}
  virtual void addPositionsObject() {}
  virtual void addUserObjects() {}
  virtual void setupComponent() {}

  // These routines can help define a component that also defines a Physics
  /// Used to add variables on a component
  virtual void addSolverVariables() {}
  /// Used to add one or more Physics to be active on the component.
  /// We recommend using the PhysicsComponentInterface instead of overriding this directly
  virtual void addPhysics() {}
  /// Used to add materials or functor materials on a component
  virtual void addMaterials() {}
  /// Used for various checks notably:
  /// - that all ICs in a ComponentInitialConditionInterface are used
  virtual void checkIntegrity() {}

  /// Use this if registering a new task to the derived ActionComponent
  virtual void actOnAdditionalTasks() {}

  /// Add a new required task for all physics deriving from this class
  /// NOTE: This does not register the task, you still need to call registerMooseAction
  void addRequiredTask(const std::string & task) { _required_tasks.insert(task); }

  /// Checks that tasks marked required by parent classes are indeed registered to derived classes
  void checkRequiredTasks() const;

  /// Get problem from action warehouse
  FEProblemBase & getProblem()
  {
    mooseAssert(_awh.problemBase().get(), "There should be a problem");
    return *_awh.problemBase().get();
  }

  /// Get the factory to build (often physics-related but not always) objects (for example a Positions)
  Factory & getFactory() const { return _factory; }

  /// Maximum dimension of the component
  unsigned int _dimension;

  /// Name(s) of the final mesh generator(s) creating the mesh for the component
  std::vector<MeshGeneratorName> _mg_names;

  /// Names of the blocks the component is comprised of
  std::vector<SubdomainName> _blocks;

  /// Names of the boundaries on the component outer surface
  std::vector<BoundaryName> _outer_boundaries;

  /// Whether the component setup should be verbose
  const bool _verbose;

  /// Manually keeps track of the tasks required by each component as tasks cannot be inherited
  std::set<std::string> _required_tasks;
};
