//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CheckLegacyParamsAction.h"

#include "AppFactory.h"
#include "Registry.h"

registerMooseAction("MooseApp", CheckLegacyParamsAction, "check_legacy_params");

InputParameters
CheckLegacyParamsAction::validParams()
{
  auto params = Action::validParams();
  params.addClassDescription("Checks whether or not objects exist that are constructed with the "
                             "legacy input parameter construction");
  return Action::validParams();
}

CheckLegacyParamsAction::CheckLegacyParamsAction(const InputParameters & params) : Action(params) {}

void
CheckLegacyParamsAction::act()
{
  // no need to do the check for sub-apps other than the zero-th of a MultiApp
  // which helps when a MultiApp spawns lots of sub-apps.
  if (_app.multiAppNumber() > 0)
    return;

  // Not a big fan of testing objects within the object itself... but this is a temp object
  // and this is the easiest way to do it. MooseTestApp uses this parameter for the test
  const bool for_test = _app.parameters().have_parameter<bool>("test_check_legacy_params") &&
                        _app.parameters().get<bool>("test_check_legacy_params");

  std::set<std::pair<std::string, std::string>> objects;

  // Get the MooseObjects and Actions whose input parameters are constructed
  // using the legacy method, skipping those registered to MooseApp
  // (which is required so that apps that use legacy params from framework
  // objects still pass while we deprecate)
  const auto add_registry_objects = [&objects, &for_test](const auto & per_label_map)
  {
    for (const auto & label_entries_pair : per_label_map)
    {
      const auto & label = label_entries_pair.first;
      const auto & entries = label_entries_pair.second;

      for (const auto & entry : entries)
      {
        const auto & object_name = entry._classname;
        if (Registry::isRegisteredObj(object_name))
        {
          const auto params = entry._params_ptr();
          if (params.fromLegacyConstruction() || for_test)
            objects.insert(std::make_pair(object_name, label));
        }
      }
    }
  };
  add_registry_objects(Registry::allObjects());
  add_registry_objects(Registry::allActions());

  // Get the applications whose input parameters are constructed using the legacy
  // method, skipping only MooseApp (which we need for now for deprecation)
  for (const auto & app_param_ptr_pair : AppFactory::instance().registeredObjectParamPointers())
  {
    const auto params = app_param_ptr_pair.second();
    if (params.fromLegacyConstruction() || for_test)
      objects.insert(std::make_pair(app_param_ptr_pair.first, ""));
  }

  if (objects.size())
  {
    std::stringstream err;
    err << "The following object(s) are constructed using the legacy input parameter "
           "construction:\n\n";
    for (const auto & object_label_pair : objects)
    {
      err << "  " << object_label_pair.first;
      if (object_label_pair.second.size())
        err << " (" << object_label_pair.second << ")";
      err << "\n";
    }

    err << "\nLegacy input parameter construction is no longer supported."
        << "\n\nConvert InputParameters validParams<T>() for each object into a static"
        << "\nmember function InputParameters T::validParams() and remove the old function."
        << "\n\nSee mooseframework.org/newsletter/2021_11.html#legacy-input-parameter-deprecation"
        << "\nfor more information.\n";
    mooseError(err.str());
  }
}
