/**
* This postprocessor demonstrates coupling a scalar variable to a postprocessor
*/

#include "ScalarCoupledPostprocessor.h"

template<>
InputParameters validParams<ScalarCoupledPostprocessor>()
{
  InputParameters params = validParams<SideIntegralPostprocessor>();
  params.addRequiredCoupledVar("variable", "Name of variable");
  params.addRequiredCoupledVar("coupled_scalar", "The name of the scalar coupled variable");
  return params;
}

ScalarCoupledPostprocessor::ScalarCoupledPostprocessor(const InputParameters & parameters) :
    SideIntegralPostprocessor(parameters),
    _coupled_scalar(coupledScalarValue("coupled_scalar")),
    _u(coupledValue("variable"))
{}

Real
ScalarCoupledPostprocessor::computeQpIntegral()
{
  return _coupled_scalar[0] - _u[_qp];
}
