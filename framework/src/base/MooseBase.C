//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseBase.h"

#include "MooseApp.h"
#include "Moose.h"
#include "Factory.h"
#include "InputParameterWarehouse.h"
#include "AppFactory.h"

const std::string MooseBase::type_param = "_type";
const std::string MooseBase::name_param = "_object_name";
const std::string MooseBase::unique_name_param = "_unique_name";
const std::string MooseBase::app_param = "_moose_app";
const std::string MooseBase::moose_base_param = "_moose_base";

InputParameters
MooseBase::validParams()
{
  InputParameters params = emptyInputParameters();
  params.addPrivateParam<std::string>(MooseBase::type_param); // The name of the class being built
  params.addPrivateParam<std::string>(MooseBase::name_param); // The name passed the factory
  params.addPrivateParam<std::string>(MooseBase::unique_name_param); // The unique name
  return params;
}

MooseBase::MooseBase(MooseApp & app, const InputParameters & params)
  : ConsoleStreamInterface(app),
    _app(app),
    _type(params.getObjectType()),
    _name(params.getObjectName()),
    _pars(params)
{
  mooseAssert(_type.size(), "Type is empty");
  mooseAssert(_name.size(), "Name is empty");

  // This enforces that we call finalizeParams (checkParams() and setting file paths)
  mooseAssert(_pars.isFinalized(), "Params are not finalized");
}

MooseBase::MooseBase(const InputParameters & params)
  : MooseBase(*params.getCheckedPointerParam<MooseApp *>(MooseBase::app_param), params)
{
}

std::string
MooseBase::typeAndName() const
{
  return type() + std::string(" \"") + name() + std::string("\"");
}

MooseObjectParameterName
MooseBase::uniqueParameterName(const std::string & parameter_name) const
{
  return MooseObjectParameterName(getBase(), name(), parameter_name);
}

MooseObjectName
MooseBase::uniqueName() const
{
  if (!_pars.have_parameter<std::string>(unique_name_param))
    mooseError("uniqueName(): Object does not have a unique name");
  return MooseObjectName(_pars.get<std::string>(unique_name_param));
}

void
MooseBase::connectControllableParams(const std::string & parameter,
                                     const std::string & object_type,
                                     const std::string & object_name,
                                     const std::string & object_parameter) const
{
  auto & factory = _app.getFactory();
  auto & ip_warehouse = _app.getInputParameterWarehouse();

  MooseObjectParameterName primary_name(uniqueName(), parameter);
  const auto base_type = factory.getValidParams(object_type).getBase();
  MooseObjectParameterName secondary_name(base_type, object_name, object_parameter);
  ip_warehouse.addControllableParameterConnection(primary_name, secondary_name);

  const auto & tags = _pars.get<std::vector<std::string>>("control_tags");
  for (const auto & tag : tags)
  {
    if (!tag.empty())
    {
      // Only adds the parameter with the different control tags if the derived class
      // properly registers the parameter to its own syntax
      MooseObjectParameterName tagged_name(tag, name(), parameter);
      ip_warehouse.addControllableParameterConnection(
          tagged_name, secondary_name, /*error_on_empty=*/false);
    }
  }
}

[[noreturn]] void
MooseBase::callMooseError(std::string msg,
                          const bool with_prefix,
                          const hit::Node * node /* = nullptr */) const
{
  callMooseError(&_app, _pars, msg, with_prefix, node);
}

[[noreturn]] void
MooseBase::callMooseError(MooseApp * const app,
                          const InputParameters & params,
                          std::string msg,
                          const bool with_prefix,
                          const hit::Node * node)
{
  if (!node)
    node = MooseBase::getHitNode(params);

  std::string multiapp_prefix = "";
  if (app)
  {
    if (!app->isUltimateMaster())
      multiapp_prefix = app->name();
    app->getOutputWarehouse().mooseConsole();
  }

  if (with_prefix)
    // False here because the hit context will get processed by the node
    msg = messagePrefix(params, false) + msg;

  moose::internal::mooseErrorRaw(msg, multiapp_prefix, node);
}

std::string
MooseBase::messagePrefix(const InputParameters & params, const bool hit_prefix)
{
  std::string prefix = "";

  if (hit_prefix)
    if (const auto node = MooseBase::getHitNode(params))
      prefix += Moose::hitMessagePrefix(*node);

  // Don't have context without type and name
  if (!params.isMooseBaseObject())
    return prefix;

  const auto & name = params.getObjectName();
  const std::string base = params.hasBase() ? params.getBase() : "object";
  const bool is_main_app = base == "Application" && name == AppFactory::main_app_name;
  prefix += "The following occurred in the ";
  if (is_main_app)
    prefix += "main " + base;
  else
    prefix += base;
  if (base != params.getObjectName() && name.size() && !is_main_app)
    prefix += " '" + name + "'";
  prefix += " of type " + params.getObjectType() + ".";
  return prefix + "\n\n";
}

const hit::Node *
MooseBase::getHitNode(const InputParameters & params)
{
  if (const auto hit_node = params.getHitNode())
    if (!hit_node->isRoot())
      return hit_node;
  return nullptr;
}
