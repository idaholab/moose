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

class PhysicsBase;

/**
 * Create a component with externally-defined Physics object active on it
 * Also serves as a base class for Components hard-coding Physics objects active
 * on themselves.
 */
class FileMeshPhysicsComponent : public FileMeshComponent
{
public:
  static InputParameters validParams();

  FileMeshPhysicsComponent(const InputParameters & parameters);

  // These objects are added by the Physics already
  virtual void addVariables() override{};
  virtual void addMooseObjects() override{};

protected:
  virtual void init() override;

  virtual std::vector<SubdomainName> getBlocks() const { return getSubdomainNames(); }
  virtual Factory & getFactory() { return getMooseApp().getFactory(); }
  virtual FEProblemBase & getProblem() { return getMooseApp().feProblem(); }
  virtual FEProblemBase & getProblem() const { return getMooseApp().feProblem(); }
  virtual const MooseMesh & getMesh() const { return constMesh(); }

  virtual std::string prefix() const { return name() + ":"; }

  // TODO: Register the variable information to the simulation
  //       Register the variable initial condition information in the simulation

  /// Physics object that creates the equations on this component
  std::vector<PhysicsBase *> _physics;
};
