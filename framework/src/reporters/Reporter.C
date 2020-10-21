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

Reporter::Reporter(const InputParameters & parameters)
  : OutputInterface(parameters),
    _reporter_params(parameters),
    _reporter_name(parameters.get<std::string>("_object_name")),
    _reporter_fe_problem(parameters.getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _reporter_data(_reporter_fe_problem->getReporterDataInternal())
{
}
