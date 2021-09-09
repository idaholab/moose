//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Reporter.h"
#include "InputParameters.h"
#include "FEProblemBase.h"

InputParameters
Reporter::validParams()
{
  InputParameters params = emptyInputParameters();
  params += OutputInterface::validParams();

  params.registerBase("Reporter");
  return params;
}

// The OutputInterface automatic creation of the "variable" build list is disabled because the
// output names for the reporters are more than just the block name. The list is updated manually
// when each value is declared.

Reporter::Reporter(const MooseObject * moose_object)
  : OutputInterface(moose_object->parameters(), /*build_list=*/false),
    _reporter_moose_object(*moose_object),
    _reporter_params(_reporter_moose_object.parameters()),
    _reporter_name(_reporter_params.get<std::string>("_object_name")),
    _reporter_fe_problem(
        *_reporter_params.getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _reporter_data(_reporter_fe_problem.getReporterData(ReporterData::WriteKey()))
{
}

void
Reporter::store(nlohmann::json & json) const
{
  json["type"] = _reporter_params.get<std::string>("_type");
}

const ReporterValueName &
Reporter::getReporterValueName(const std::string & param_name) const
{
  if (!_reporter_params.isParamValid(param_name))
    _reporter_moose_object.mooseError(
        "When getting a ReporterValueName, failed to get a parameter with the name \"",
        param_name,
        "\".",
        "\n\nKnown parameters:\n",
        _reporter_moose_object.parameters());

  if (_reporter_params.isType<ReporterValueName>(param_name))
    return _reporter_params.get<ReporterValueName>(param_name);

  _reporter_moose_object.mooseError("Supplied parameter with name \"",
                                    param_name,
                                    "\" of type \"",
                                    _reporter_params.type(param_name),
                                    "\" is not an expected type for getting a Reporter value.\n\n",
                                    "The expected type is \"ReporterValueName\".");
}
