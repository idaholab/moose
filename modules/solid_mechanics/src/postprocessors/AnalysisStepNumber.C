//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AnalysisStepNumber.h"
#include "UserObjectInterface.h"

registerMooseObject("SolidMechanicsApp", AnalysisStepNumber);

InputParameters
AnalysisStepNumber::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addRequiredParam<UserObjectName>("analysis_step_user_object",
                                          "The AnalysisStepUserObject that stores step times.");
  params.addParam<bool>("use_one_based_indexing", false, "Make step number start at one.");
  params.addClassDescription("Outputs the current analysis step number.");
  params.set<ExecFlagEnum>("execute_on") = {
      EXEC_INITIAL, EXEC_TIMESTEP_BEGIN}; // only need to execute once per time step
  return params;
}

AnalysisStepNumber::AnalysisStepNumber(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _step_uo(getUserObject<AnalysisStepUserObject>("analysis_step_user_object")),
    _use_one_based_indexing(getParam<bool>("use_one_based_indexing"))
{
}

Real
AnalysisStepNumber::getValue() const
{
  return _step_uo.getStep(_t_old) + int(_use_one_based_indexing);
}
