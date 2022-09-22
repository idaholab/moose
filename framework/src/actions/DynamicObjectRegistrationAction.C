//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DynamicObjectRegistrationAction.h"
#include "Factory.h"
#include "FEProblem.h"
#include "MooseApp.h"

registerMooseAction("MooseApp", DynamicObjectRegistrationAction, "dynamic_object_registration");

InputParameters
DynamicObjectRegistrationAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription("Register MooseObjects from other applications dynamically.");
  params.addParam<std::vector<std::string>>("register_objects_from",
                                            "The names of other applications from which objects "
                                            "will be registered from (dynamic registration).");
  params.addParam<std::vector<std::string>>(
      "object_names", "The names of the objects to register (Default: register all).");
  params.addParam<std::string>("library_path",
                               "",
                               "Path to search for dynamic libraries (please "
                               "avoid committing absolute paths in addition to "
                               "MOOSE_LIBRARY_PATH)");
  params.addParam<std::string>(
      "library_name",
      "",
      "The file name of the library (*.la file) that will be dynamically loaded.");
  return params;
}

DynamicObjectRegistrationAction::DynamicObjectRegistrationAction(const InputParameters & parameters)
  : Action(parameters)
{
  /**
   * Dynamic object registration must occur before parsing. The parser needs to retrieve parameters
   * for each
   * registered object that it must build.
   */
  if (isParamValid("register_objects_from"))
  {
    // Only register the requested objects
    if (isParamValid("object_names"))
      _factory.restrictRegisterableObjects(getParam<std::vector<std::string>>("object_names"));

    std::vector<std::string> application_names =
        getParam<std::vector<std::string>>("register_objects_from");
    for (const auto & app_name : application_names)
    {
      _app.dynamicAllRegistration(app_name,
                                  &_factory,
                                  &_action_factory,
                                  &_awh.syntax(),
                                  getParam<std::string>("library_path"),
                                  getParam<std::string>("library_name"));
    }
  }
}

void
DynamicObjectRegistrationAction::act()
{
}
