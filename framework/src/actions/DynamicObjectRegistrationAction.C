/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "DynamicObjectRegistrationAction.h"
#include "Factory.h"
#include "FEProblem.h"
#include "MooseApp.h"

template <>
InputParameters
validParams<DynamicObjectRegistrationAction>()
{
  InputParameters params = validParams<Action>();

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
  return params;
}

DynamicObjectRegistrationAction::DynamicObjectRegistrationAction(InputParameters parameters)
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
      _app.dynamicObjectRegistration(app_name, &_factory, getParam<std::string>("library_path"));
      _app.dynamicSyntaxAssociation(
          app_name, &_awh.syntax(), &_action_factory, getParam<std::string>("library_path"));
    }
  }
}

void
DynamicObjectRegistrationAction::act()
{
}
