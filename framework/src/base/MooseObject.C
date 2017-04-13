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

// MOOSE includes
#include "MooseObject.h"
#include "MooseApp.h"
#include "MooseUtils.h"

template <>
InputParameters
validParams<MooseObject>()
{
  InputParameters params;
  params.addParam<bool>("enable", true, "Set the enabled status of the MooseObject.");
  params.addParam<std::vector<std::string>>(
      "control_tags",
      "Adds user-defined labels for accessing object parameters via control logic.");
  params.addPrivateParam<std::string>("_object_name"); // the name passed to Factory::create
  params.addParamNamesToGroup("enable control_tags", "Advanced");

  return params;
}

MooseObject::MooseObject(const InputParameters & parameters)
  : ConsoleStreamInterface(
        *parameters.get<MooseApp *>("_moose_app")), // Can't call getParam before pars is set
    ParallelObject(
        *parameters.get<MooseApp *>("_moose_app")), // Can't call getParam before pars is set
    _app(*parameters.getCheckedPointerParam<MooseApp *>("_moose_app")),
    _pars(parameters),
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
