//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralUserObject.h"

#include "libmesh/system.h"

/**
 * Attaches a libMesh constraint object that adds no constraints to the nonlinear system on
 * construction, to test that exact constraints refuse to replace one.
 */
class AttachDummySystemConstraint : public GeneralUserObject
{
public:
  static InputParameters validParams();

  AttachDummySystemConstraint(const InputParameters & params);

  virtual void initialize() override {}
  virtual void execute() override {}
  virtual void finalize() override {}

private:
  /// A libMesh constraint object that adds no constraints
  class DummyConstraint : public libMesh::System::Constraint
  {
  public:
    virtual void constrain() override {}
  };

  /// The constraint object attached to the nonlinear system
  DummyConstraint _dummy_constraint;
};
