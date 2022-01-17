#include "BoolControlDataValuePostprocessor.h"
#include "THMProblem.h"

registerMooseObject("ThermalHydraulicsApp", BoolControlDataValuePostprocessor);

InputParameters
BoolControlDataValuePostprocessor::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addRequiredParam<std::string>("control_data_name",
                                       "The name of the control data to output.");
  return params;
}

BoolControlDataValuePostprocessor::BoolControlDataValuePostprocessor(
    const InputParameters & parameters)
  : GeneralPostprocessor(parameters), _control_data_name(getParam<std::string>("control_data_name"))
{
  THMProblem * thm_problem =
      dynamic_cast<THMProblem *>(getCheckedPointerParam<FEProblemBase *>("_fe_problem_base"));
  if (thm_problem)
  {
    _thm_problem = thm_problem;
    _control_data_value = _thm_problem->getControlData<bool>(_control_data_name);
  }
  else
    mooseError(name(),
               ": Cannot use BoolControlDataValuePostprocessor without the component system.");
}

void
BoolControlDataValuePostprocessor::initialize()
{
}

void
BoolControlDataValuePostprocessor::execute()
{
}

Real
BoolControlDataValuePostprocessor::getValue()
{
  if (_control_data_value->get())
    return 1.;
  else
    return 0.;
}
