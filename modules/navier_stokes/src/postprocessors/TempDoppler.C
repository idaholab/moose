#include "TempDoppler.h"
#include "MooseMesh.h"
#include "SubProblem.h"

registerMooseObject("NavierStokesApp", TempDoppler);

InputParameters
TempDoppler::validParams()
{
  InputParameters params = ElementIntegralVariablePostprocessor::validParams();
  params.addClassDescription("Calculates the temperature feedback due to Doppler broadening depending on the Temperature in the given cell.");
  params.addRequiredCoupledVar("T_ref", "The refference temperature");
  params.addRequiredCoupledVar("flux", "The fundamental shape");
  params.addRequiredCoupledVar("adjflux", "The adjoint fundamental shape");
  params.addParam<Real>("a", 1, "a in <alog(bT_i+c)+d>");
  params.addParam<Real>("b", 1, "b in <alog(bT_i+c)+d>");
  params.addParam<Real>("c", 0, "c in <alog(bT_i+c)+d>");
  params.addParam<Real>("d", 0, "d in <alog(bT_i+c)+d>");
  return params;
}

TempDoppler::TempDoppler(const InputParameters & parameters)
  : ElementIntegralVariablePostprocessor(parameters),
	_T_ref(coupledValue("T_ref")),
	_flux(coupledValue("flux")),
	_adjflux(coupledValue("adjflux")),
	_a(getParam<Real>("a")),
	_b(getParam<Real>("b")),
	_c(getParam<Real>("c")),
	_d(getParam<Real>("d"))
{
}



Real
TempDoppler::getValue() const
{
	return ElementIntegralVariablePostprocessor::getValue();
}

Real
TempDoppler::computeQpIntegral()
{
	 return _adjflux[_qp]*_flux[_qp] * (_u[_qp]-_T_ref[_qp])* (_a*log(_b*_u[_qp]+_c)+_d);
}

