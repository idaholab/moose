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

class FEProblem;
class FEProblemBase;
class EigenProblem;
class SubProblem;
class SystemBase;
class AuxiliarySystem;
class Transient;

std::string
paramErrorPrefix(const InputParameters & params, const std::string & param)
{
  return params.errorPrefix(param);
}

InputParameters
MooseObject::validParams()
{
  InputParameters params = emptyInputParameters();
  params.addParam<bool>("enable", true, "Set the enabled status of the MooseObject.");
  params.addParam<std::vector<std::string>>(
      "control_tags",
      "Adds user-defined labels for accessing object parameters via control logic.");
  params.addParamNamesToGroup("enable control_tags", "Advanced");
  params.addPrivateParam<std::string>("_type");        // The name of the class being built
  params.addPrivateParam<std::string>("_object_name"); // The name passed to Factory::create
  params.addPrivateParam<std::string>("_unique_name"); // The unique name generated in the warehouse
  params.addPrivateParam<FEProblem *>("_fe_problem", nullptr);
  params.addPrivateParam<FEProblemBase *>("_fe_problem_base", nullptr);
  params.addPrivateParam<EigenProblem *>("_eigen_problem", nullptr);
  params.addPrivateParam<SubProblem *>("_subproblem", nullptr);
  params.addPrivateParam<SystemBase *>("_sys", nullptr);
  params.addPrivateParam<SystemBase *>("_nl_sys", nullptr);
  params.addPrivateParam<Transient *>("_executioner", nullptr);
  params.addPrivateParam<THREAD_ID>("_tid");
  params.addPrivateParam<bool>("_residual_object", false);
  return params;
}

MooseObject::MooseObject(const InputParameters & parameters)
  : ConsoleStreamInterface(*parameters.getCheckedPointerParam<MooseApp *>("_moose_app")),
    ParallelObject(*parameters.getCheckedPointerParam<MooseApp *>("_moose_app")),
    DataFileInterface<MooseObject>(*this),
    _pars(parameters),
    _app(*getCheckedPointerParam<MooseApp *>("_moose_app")),
    _type(getParam<std::string>("_type")),
    _name(getParam<std::string>("_object_name")),
    _enabled(getParam<bool>("enable"))
{
}

[[noreturn]] void
callMooseErrorRaw(std::string & msg, MooseApp * app)
{
  app->getOutputWarehouse().mooseConsole();
  std::string prefix;
  if (!app->isUltimateMaster())
    prefix = app->name();
  moose::internal::mooseErrorRaw(msg, prefix);
}

std::string
MooseObject::errorPrefix(const std::string & error_type) const
{
  std::stringstream oss;
  oss << "The following " << error_type << " occurred in the object \"" << name()
      << "\", of type \"" << type() << "\".\n\n";
  return oss.str();
}

std::string
MooseObject::typeAndName() const
{
  return type() + std::string(" \"") + name() + std::string("\"");
}
