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
#include "MooseVerbosityHelper.h"

const std::string MooseBase::type_param = "_type";
const std::string MooseBase::name_param = "_object_name";
const std::string MooseBase::unique_name_param = "_unique_name";
const std::string MooseBase::app_param = "_moose_app";
const std::string MooseBase::moose_base_param = "_moose_base";
#ifdef MOOSE_KOKKOS_ENABLED
const std::string MooseBase::kokkos_object_param = "_kokkos_object";
#endif

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
    MooseVerbosityHelper(this, params),
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

std::string
MooseVerbosityHelper::messagePrefix(const bool hit_prefix) const
{
  return messagePrefix(_moose_base.parameters(), hit_prefix);
}
