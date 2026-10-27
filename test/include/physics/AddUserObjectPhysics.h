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
 * Test Physics that adds a UserObject. Optionally, the added UserObject resolves another UserObject
 * (named through the 'dependency' parameter) in its constructor. The added UserObject is also
 * declared through getSuppliedUserObjects() so that other objects may depend on it. Used to
 * demonstrate and regression-test UserObject construction ordering between a Physics and the
 * [UserObjects] block, in both directions (idaholab/moose#7879).
 */
class AddUserObjectPhysics : public PhysicsBase
{
public:
  static InputParameters validParams();

  AddUserObjectPhysics(const InputParameters & parameters);

  virtual std::vector<UserObjectName> getSuppliedUserObjects() const override;

private:
  virtual void addUserObjects() override;
};
