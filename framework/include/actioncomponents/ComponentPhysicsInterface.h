//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ActionComponent.h"
#include "InputParameters.h"
#include "MooseTypes.h"
#include "PhysicsBase.h"

/**
 * Helper class to help creating an entire physics
 * Note: Trying out virtual inheritance. It makes things
 *       a little easier to define as we can use the attributes
 *       of the underlying ActionComponent
 */
class ComponentPhysicsInterface : public virtual ActionComponent
{
public:
  static InputParameters validParams();

  ComponentPhysicsInterface(const InputParameters & params);

  /// Get the Physics from the Component
  std::vector<PhysicsBase *> getPhysics() const { return _physics; }

protected:
  /// Whether the physics exists with the requested type and name
  template <typename T>
  bool physicsExists(const PhysicsName & name) const;

  virtual void addPhysics() override;

  /// Names of the Physics defined on the component
  std::vector<PhysicsName> _physics_names;
  /// Pointers to the Physics defined on the component
  std::vector<PhysicsBase *> _physics;
};

template <typename T>
bool
ComponentPhysicsInterface::physicsExists(const PhysicsName & name) const
{
  const auto physics = _awh.getPhysics<const T>();
  for (const auto phys : physics)
    if (phys->name() == name)
      return true;
  return false;
}
