//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "MooseBaseParameterInterface.h"
#include "MooseObjectParameterName.h"
#include "MooseApp.h"
#include "MooseUtils.h"
#include "MooseBase.h"
#include "InputParameterWarehouse.h"

std::string
paramErrorPrefix(const InputParameters & params, const std::string & param)
{
  return params.errorPrefix(param);
}

MooseBaseParameterInterface::MooseBaseParameterInterface(const InputParameters & parameters,
                                                         const MooseBase * const base)
  : _pars(parameters),
    _factory(base->getMooseApp().getFactory()),
    _action_factory(base->getMooseApp().getActionFactory()),
    _moose_base(base)
{
}

void
MooseBaseParameterInterface::connectControllableParams(const std::string & parameter,
                                                       const std::string & object_type,
                                                       const std::string & object_name,
                                                       const std::string & object_parameter) const
{
  MooseObjectParameterName primary_name(uniqueName(), parameter);
  const auto base_type = _factory.getValidParams(object_type).get<std::string>("_moose_base");
  MooseObjectParameterName secondary_name(base_type, object_name, object_parameter);
  _moose_base->getMooseApp().getInputParameterWarehouse().addControllableParameterConnection(
      primary_name, secondary_name);

  const auto & tags = _pars.get<std::vector<std::string>>("control_tags");
  for (const auto & tag : tags)
  {
    if (!tag.empty())
    {
      MooseObjectParameterName tagged_name(tag, _moose_base->name(), parameter);
      _moose_base->getMooseApp().getInputParameterWarehouse().addControllableParameterConnection(
          tagged_name, secondary_name);
    }
  }
}

std::string
MooseBaseParameterInterface::objectErrorPrefix(const std::string & error_type) const
{
  std::stringstream oss;
  oss << "The following " << error_type << " occurred in the class \"" << _moose_base->name()
      << "\", of type \"" << _moose_base->type() << "\".\n\n";
  return oss.str();
}
