//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PhysicsBase.h"

/**
 * Test Physics that adds a UserObject which resolves another UserObject (named through the
 * 'dependency' parameter) in its constructor. Used to demonstrate and regression-test UserObject
 * construction ordering between a Physics and the [UserObjects] block (idaholab/moose#7879).
 */
class AddUserObjectPhysics : public PhysicsBase
{
public:
  static InputParameters validParams();

  AddUserObjectPhysics(const InputParameters & parameters);

private:
  virtual void addUserObjects() override;
};
