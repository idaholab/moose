#include "RealControlDataValuePostprocessor.h"
#include "THMProblem.h"

registerMooseObject("THMApp", RealControlDataValuePostprocessor);

InputParameters
RealControlDataValuePostprocessor::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addRequiredParam<std::string>("control_data_name",
                                       "The name of the control data to output.");
  return params;
}

RealControlDataValuePostprocessor::RealControlDataValuePostprocessor(
    const InputParameters & parameters)
  : GeneralPostprocessor(parameters), _control_data_name(getParam<std::string>("control_data_name"))
{
  THMProblem * thm_problem =
      dynamic_cast<THMProblem *>(getCheckedPointerParam<FEProblemBase *>("_fe_problem_base"));
  if (thm_problem)
  {
    _thm_problem = thm_problem;
    _control_data_value = _thm_problem->getControlData<Real>(_control_data_name);
  }
  else
    mooseError(name(),
               ": Cannot use RealControlDataValuePostprocessor without the component system.");
}

void
RealControlDataValuePostprocessor::initialize()
{
}

void
RealControlDataValuePostprocessor::execute()
{
}

Real
RealControlDataValuePostprocessor::getValue()
{
  return _control_data_value->get();
}
