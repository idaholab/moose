#include "THMSolvePostprocessorControl.h"

registerMooseObject("THMApp", THMSolvePostprocessorControl);

InputParameters
THMSolvePostprocessorControl::validParams()
{
  InputParameters params = THMControl::validParams();
  params.addRequiredParam<PostprocessorName>(
      "postprocessor", "The name of the postprocessot that indicates is a solve should be done.");
  params.addClassDescription("Control the solve based on a postprocessor value");
  return params;
}

THMSolvePostprocessorControl::THMSolvePostprocessorControl(const InputParameters & parameters)
  : THMControl(parameters), _solve_pps(getPostprocessorValue("postprocessor"))
{
}

void
THMSolvePostprocessorControl::execute()
{
  setControllableValueByName<bool>("Problem::" + _fe_problem.name(), "solve", _solve_pps != 0.);
}
