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

Reporter::Reporter(const InputParameters & parameters)
  : OutputInterface(parameters, /*build_list=*/false),
    _reporter_params(parameters),
    _reporter_name(parameters.get<std::string>("_object_name")),
    _reporter_fe_problem(*parameters.getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _reporter_data(_reporter_fe_problem.getReporterData(ReporterData::WriteKey()))
{
}

void
Reporter::store(nlohmann::json & json) const
{
  json["name"] = _reporter_name;
  json["type"] = _reporter_params.get<std::string>("_type");
}
