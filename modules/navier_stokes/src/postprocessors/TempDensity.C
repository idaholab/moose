#include "TempDensity.h"
#include "MooseMesh.h"
#include "SubProblem.h"

registerMooseObject("NavierStokesApp", TempDensity);

InputParameters
TempDensity::validParams()
{
  InputParameters params = ElementIntegralVariablePostprocessor::validParams();
  params.addClassDescription("Calculates the temperature feedback due to Density broadening depending on the Temperature in the given cell.");
  params.addRequiredCoupledVar("T_ref", "The refference temperature");
  params.addRequiredCoupledVar("flux", "The fundamental shape");
  params.addParam<Real>("a", 1, "a in <aT_i+b>");
  params.addParam<Real>("b", 0, "b in <aT_i+b>");
  return params;
}

TempDensity::TempDensity(const InputParameters & parameters)
  : ElementIntegralVariablePostprocessor(parameters),
	_T_ref(coupledValue("T_ref")),
	_flux(coupledValue("flux")),
	_a(getParam<Real>("a")),
	_b(getParam<Real>("b"))
{
}



Real
TempDensity::getValue() const
{
	return ElementIntegralVariablePostprocessor::getValue();
}

Real
TempDensity::computeQpIntegral()
{
	 return _flux[_qp] * (_u[_qp]-_T_ref[_qp])* (_a*_u[_qp]+_b);
}
