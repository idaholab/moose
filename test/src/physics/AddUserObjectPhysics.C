//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddUserObjectPhysics.h"

registerMooseAction("MooseTestApp", AddUserObjectPhysics, "add_user_object");
registerPhysicsBaseTasks("MooseTestApp", AddUserObjectPhysics);

InputParameters
AddUserObjectPhysics::validParams()
{
  InputParameters params = PhysicsBase::validParams();
  params.addClassDescription("Test Physics that adds a UserObject, optionally depending on another "
                             "UserObject at construction.");
  params.addParam<UserObjectName>("dependency",
                                  "Name of a UserObject that the UserObject added by this Physics "
                                  "resolves in its constructor. If unset, the added UserObject has "
                                  "no dependency and only serves as one other objects may depend "
                                  "on.");
  return params;
}

AddUserObjectPhysics::AddUserObjectPhysics(const InputParameters & parameters)
  : PhysicsBase(parameters)
{
}

std::vector<UserObjectName>
AddUserObjectPhysics::getSuppliedUserObjects() const
{
  return {name() + "_uo"};
}

void
AddUserObjectPhysics::addUserObjects()
{
  auto params = getFactory().getValidParams("UserObjectInterfaceTest");
  if (isParamValid("dependency"))
  {
    params.set<UserObjectName>("uo") = getParam<UserObjectName>("dependency");
    // 'has' makes the added UserObject check, in its constructor, that 'dependency' already exists;
    // it errors if the dependency has not been constructed yet.
    params.set<bool>("has") = true;
  }
  addUserObject("UserObjectInterfaceTest", name() + "_uo", params);
}
