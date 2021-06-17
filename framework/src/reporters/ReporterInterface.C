//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ReporterInterface.h"

#include "FEProblemBase.h"

InputParameters
ReporterInterface::validParams()
{
  return emptyInputParameters();
}

ReporterInterface::ReporterInterface(const MooseObject * moose_object)
  : _ri_params(moose_object->parameters()),
    _ri_fe_problem_base(*_ri_params.getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _ri_reporter_data(_ri_fe_problem_base.getReporterData()),
    _ri_moose_object(*moose_object)
{
}

bool
ReporterInterface::hasReporterValue(const std::string & param_name) const
{
  if (!reportersAdded())
    _ri_moose_object.mooseError(
        "Cannot call hasReporterValue() until all Reporters have been constructed.");

  return hasReporterValueByName(getReporterName(param_name));
}

bool
ReporterInterface::hasReporterValueByName(const ReporterName & reporter_name) const
{
  if (!reportersAdded())
    _ri_moose_object.mooseError(
        "Cannot call hasReporterValueByName() until all Reporters have been constructed.");

  return _ri_reporter_data.hasReporterValue(reporter_name);
}

const ReporterName &
ReporterInterface::getReporterName(const std::string & param_name) const
{
  if (!_ri_params.isParamValid(param_name))
    _ri_moose_object.mooseError(
        "When getting a Reporter, failed to get a parameter with the name \"",
        param_name,
        "\".",
        "\n\nKnown parameters:\n",
        _ri_moose_object.parameters());

  if (_ri_params.isType<ReporterName>(param_name))
    return _ri_params.get<ReporterName>(param_name);

  _ri_moose_object.mooseError("Supplied parameter with name \"",
                              param_name,
                              "\" of type \"",
                              _ri_params.type(param_name),
                              "\" is not an expected type for getting a Reporter.\n\n",
                              "The expected type is \"ReporterName\".");
}

bool
ReporterInterface::reportersAdded() const
{
  return _ri_fe_problem_base.getMooseApp().actionWarehouse().isTaskComplete("add_reporter");
}
