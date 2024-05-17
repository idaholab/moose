//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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

class PhysicsBase;
class FEProblem;

/**
 * Base class for components that are defined using an action
 */
class ComponentAction : public Action
{
public:
  /**
   * Class constructor
   */
  static InputParameters validParams();

  ComponentAction(const InputParameters & params);

  virtual void act() override final;

  /// Get the name of the mesh generator created by this component that generates the mesh for it
  const MeshGeneratorName & meshName() const { return _mg_name; }

  /// Return the outer surface boundaries
  virtual const std::vector<BoundaryName> & outerSurfaceBoundaries() const
  {
    return _outer_boundaries;
  };

  /// Return the component volume
  virtual Real volume() const { mooseError("Not implemented"); }

  /// Return the component outer boundary area
  virtual Real outerSurfaceArea() const { mooseError("Not implemented"); }

protected:
  // The default implementation of these routines will do nothing as we do not expect all Components
  // to be defining an object of every type
  // These routines are to help define a strictly geometrical component
  virtual void addMeshGenerators() {}
  virtual void addPositionsObject() {}
  virtual void addUserObjects() {}
  virtual void setupComponent() {}

  // These routines can help define a component that also defines a Physics
  virtual void addNonlinearVariables() {}

  /// Use this if registering a new task to the derived ComponentAction
  virtual void actOnAdditionalTasks() {};

  virtual void addPhysics() {};

  /// Get problem from action warehouse
  FEProblem & getProblem()
  {
    mooseAssert(_awh.problem().get(), "There should be a problem");
    return *_awh.problem().get();
  }

  /// Maximum dimension of the component
  unsigned int _dimension;

  /// Name of the final mesh generator creating the mesh for the component
  MeshGeneratorName _mg_name;

  /// Names of the boundaries on the component outer surface
  std::vector<BoundaryName> _outer_boundaries;

  /// Whether the component setup should be verbose
  const bool _verbose;
};
