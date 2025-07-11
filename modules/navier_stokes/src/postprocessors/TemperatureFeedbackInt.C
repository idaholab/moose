#include "TemperatureFeedbackInt.h"
#include "MooseMesh.h"
#include "SubProblem.h"

registerMooseObject("NavierStokesApp", TemperatureFeedbackInt);

InputParameters
TemperatureFeedbackInt::validParams()
{
  InputParameters params = ElementIntegralVariablePostprocessor::validParams();
  params.addClassDescription("Calculates the temperature feedback depending on the Temperature");
  params.addRequiredCoupledVar("T_ref", "The refference temperature");
  params.addRequiredCoupledVar("flux", "The refference reactivity shape");
  params.addParam<Real>("total_rho", 0.0, "the total change in rho [1/K]");
  params.addParam<PostprocessorName>("Norm", 1.0, "the scalar product <flux|flux>");
  return params;
}

TemperatureFeedbackInt::TemperatureFeedbackInt(const InputParameters & parameters)
  : ElementIntegralVariablePostprocessor(parameters),
	_T_ref(coupledValue("T_ref")),
	_flux(coupledValue("flux")),
	_Norm(getPostprocessorValue("Norm")),
	_total_rho(getParam<Real>("total_rho"))
{
}



Real
TemperatureFeedbackInt::getValue() const
{
	return ElementIntegralVariablePostprocessor::getValue();
}

Real
TemperatureFeedbackInt::computeQpIntegral()
{
	 return _flux[_qp] * (_u[_qp]-_T_ref[_qp])* _total_rho/_Norm;
}

