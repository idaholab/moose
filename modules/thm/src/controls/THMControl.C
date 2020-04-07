#include "THMControl.h"

InputParameters
THMControl::validParams()
{
  InputParameters params = Control::validParams();
  params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_TIMESTEP_BEGIN};
  params.addPrivateParam<THMProblem *>("_thm_problem");
  params.suppressParameter<ExecFlagEnum>("execute_on");
  return params;
}

THMControl::THMControl(const InputParameters & parameters)
  : Control(parameters), _sim(getParam<THMProblem *>("_thm_problem"))
{
}
