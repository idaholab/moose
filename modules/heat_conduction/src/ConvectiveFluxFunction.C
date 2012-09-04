#include "ConvectiveFluxFunction.h"

#include "Function.h"

template<>
InputParameters validParams<ConvectiveFluxFunction>()
{
  InputParameters params = validParams<IntegratedBC>();
  params.addRequiredParam<FunctionName>("T_infinity", "Function describing far-field temperature");
  params.addRequiredParam<Real>("coefficient", "Heat transfer coefficient");

  return params;
}

ConvectiveFluxFunction::ConvectiveFluxFunction(const std::string & name, InputParameters parameters) :
    IntegratedBC(name, parameters),
    _T_infinity(getFunction("T_infinity")),
    _coefficient(getParam<Real>("coefficient"))
{}


Real
ConvectiveFluxFunction::computeQpResidual()
{
  return _test[_i][_qp] *_coefficient * (_u[_qp] - _T_infinity.value(_t, _q_point[_qp]));
}

Real
ConvectiveFluxFunction::computeQpJacobian()
{
  return _test[_i][_qp] * _coefficient * _phi[_j][_qp];
}
