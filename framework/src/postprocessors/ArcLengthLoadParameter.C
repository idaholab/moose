//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ArcLengthLoadParameter.h"

#include "ArcLengthProblem.h"

registerMooseObject("MooseApp", ArcLengthLoadParameter);

InputParameters
ArcLengthLoadParameter::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addClassDescription(
      "Reports the load parameter of the arc-length continuation performed by ArcLengthProblem.");

  ExecFlagEnum & exec_enum = params.set<ExecFlagEnum>("execute_on", true);
  exec_enum = {EXEC_ARC_LENGTH_INCREMENT, EXEC_TIMESTEP_END};
  params.setDocString("execute_on", exec_enum.getDocString());

  return params;
}

ArcLengthLoadParameter::ArcLengthLoadParameter(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _arc_length_problem(dynamic_cast<const ArcLengthProblem *>(&_fe_problem))
{
  if (!_arc_length_problem)
    mooseError("The reported load parameter is held by the arc-length problem, so this "
               "postprocessor requires [Problem] type = ArcLengthProblem.");
}

Real
ArcLengthLoadParameter::getValue() const
{
  return _arc_length_problem->loadParameter();
}
