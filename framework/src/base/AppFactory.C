//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AppFactory.h"
#include "MooseApp.h"

AppFactory &
AppFactory::instance()
{
  // We need a naked new here (_not_ a smart pointer or object instance) due to what seems like a
  // bug in clang's static object destruction when using dynamic library loading.
  static AppFactory * instance = nullptr;
  if (!instance)
    instance = new AppFactory;
  return *instance;
}

InputParameters
AppFactory::getValidParams(const std::string & name)
{
  if (const auto it = _name_to_build_info.find(name); it != _name_to_build_info.end())
  {
    auto params = it->second->buildParameters();
    params.set<std::string>("_type") = name;
    return params;
  }

  mooseError(std::string("A '") + name + "' is not a registered object\n\n");
}

MooseAppPtr
AppFactory::createShared(InputParameters parameters)
{
  const auto & type = parameters.get<std::string>("_type");

  // Error if the application type is not located
  const auto it = _name_to_build_info.find(type);
  if (it == _name_to_build_info.end())
    mooseError("Object '" + type + "' was not registered.");
  auto & build_info = it->second;

  build_info->_app_creation_count++;

  _currently_constructing = true; // force app construction via AppFactory
  auto app = build_info->build(parameters);
  _currently_constructing = false;

  return app;
}

std::size_t
AppFactory::createdAppCount(const std::string & app_type) const
{
  // Error if the application type is not located
  const auto it = _name_to_build_info.find(app_type);
  if (it == _name_to_build_info.end())
    mooseError("AppFactory::createdAppCount(): '", app_type, "' is not a registered app");

  return it->second->_app_creation_count;
}
