//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "MooseObject.h"
#include "MooseApp.h"
#include "MooseUtils.h"

template <>
InputParameters
validParams<MooseObject>()
{
  InputParameters params = emptyInputParameters();
  params.addParam<bool>("enable", true, "Set the enabled status of the MooseObject.");
  params.addParam<std::vector<std::string>>(
      "control_tags",
      "Adds user-defined labels for accessing object parameters via control logic.");
  params.addPrivateParam<std::string>("_object_name"); // the name passed to Factory::create
  params.addParamNamesToGroup("enable control_tags", "Advanced");

  return params;
}

MooseObject::MooseObject(const InputParameters & parameters)
  : ConsoleStreamInterface(*parameters.getCheckedPointerParam<MooseApp *>("_moose_app")),
    ParallelObject(*parameters.getCheckedPointerParam<MooseApp *>("_moose_app")),
    _pars(parameters),
    _app(*getCheckedPointerParam<MooseApp *>("_moose_app")),
    _name(getParam<std::string>("_object_name")),
    _enabled(getParam<bool>("enable")){}

        [[noreturn]] void callMooseErrorRaw(std::string & msg, MooseApp * app)
{
  app->getOutputWarehouse().mooseConsole();
  std::string prefix;
  if (!app->isUltimateMaster())
    prefix = app->name();
  moose::internal::mooseErrorRaw(msg, prefix);
}
