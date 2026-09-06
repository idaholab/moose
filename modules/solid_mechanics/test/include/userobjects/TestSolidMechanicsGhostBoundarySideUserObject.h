//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#pragma once

#include "SideUserObject.h"

// Based on TestGhostBoundarySideUserObject in the framework test application.
class TestSolidMechanicsGhostBoundarySideUserObject : public SideUserObject
{
public:
  static InputParameters validParams();

  TestSolidMechanicsGhostBoundarySideUserObject(const InputParameters & parameters);
  virtual void initialize() override {}
  virtual void execute() override {}
  virtual void finalize() override {}
  virtual void threadJoin(const UserObject &) override {}
};
