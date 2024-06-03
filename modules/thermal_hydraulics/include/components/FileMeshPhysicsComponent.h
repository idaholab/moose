//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FileMeshComponent.h"
#include "THMMesh.h"

/**
 * Create a component with user-selected Physics active on it
 */
class FileMeshPhysicsComponent : public FileMeshComponent
{
public:
  static InputParameters validParams();

  FileMeshPhysicsComponent(const InputParameters & parameters);

  virtual void addRelationshipManagers(Moose::RelationshipManagerType input_rm_type) override;
  // These objects are added by the Physics already
  virtual void addVariables() override{};
  virtual void addMooseObjects() override{};

protected:
  virtual void init() override;

  /// Return the blocks this component defines (assuming the ids do not overlap with other components)
  virtual std::vector<SubdomainName> getBlocks() const { return getSubdomainNames(); }
  virtual Factory & getFactory() { return getMooseApp().getFactory(); }
  virtual FEProblemBase & getProblem() { return getMooseApp().feProblem(); }
  /// Returns the mesh, which also contains other components
  virtual const MooseMesh & getMesh() const { return constMesh(); }
  /// Returns a useful prefix for logs
  virtual std::string prefix() const { return name() + ":"; }

private:
  /// Physics that creates the equations on this component
  std::vector<PhysicsBase *> _physics;
};
