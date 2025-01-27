//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseBaseParameterInterface.h"

#include "MooseApp.h"
#include "InputParameterWarehouse.h"

std::string
paramErrorPrefix(const InputParameters & params, const std::string & param)
{
  return params.errorPrefix(param);
}

MooseBaseParameterInterface::MooseBaseParameterInterface(const MooseBase & base,
                                                         const InputParameters & parameters)
  : _pars(parameters),
    _factory(base.getMooseApp().getFactory()),
    _action_factory(base.getMooseApp().getActionFactory()),
    _moose_base(base)
{
  // This enforces that we call finalizeParams (checkParams() and setting file paths)
  mooseAssert(_pars.isFinalized(), "Params are not finalized");
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
  _moose_base.getMooseApp().getInputParameterWarehouse().addControllableParameterConnection(
      primary_name, secondary_name);

  const auto & tags = _pars.get<std::vector<std::string>>("control_tags");
  for (const auto & tag : tags)
  {
    if (!tag.empty())
    {
      // Only adds the parameter with the different control tags if the derived class
      // properly registers the parameter to its own syntax
      MooseObjectParameterName tagged_name(tag, _moose_base.name(), parameter);
      _moose_base.getMooseApp().getInputParameterWarehouse().addControllableParameterConnection(
          tagged_name, secondary_name, /*error_on_empty=*/false);
    }
  }
}
