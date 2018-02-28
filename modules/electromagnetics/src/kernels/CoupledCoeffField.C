#include "CoupledCoeffField.h"
#include "Function.h"

template <> InputParameters validParams<CoupledCoeffField>() {
  InputParameters params = validParams<Kernel>();
  params.addParam<Real>("k", 1.0, "Coefficient multiplier (to be squared) for field.");
  params.addParam<FunctionName>("func",1.0,"Function multiplier for field.");
  params.addRequiredCoupledVar("coupled_field","Coupled field variable.");
  params.addParam<Real>("sign",1.0,"Sign of term in weak form.");
  return params;
}

CoupledCoeffField::CoupledCoeffField(const InputParameters & parameters)
    : Kernel(parameters),

      _coefficient(getParam<Real>("k")),

      _func(getFunction("func")),

      _coupled_val(coupledValue("coupled_field")),

      _sign(getParam<Real>("sign"))

{}

Real CoupledCoeffField::computeQpResidual() {
  return _sign * std::pow(_coefficient,2) * _func.value(_t,_q_point[_qp]) * _test[_i][_qp] * _coupled_val[_qp];
}

Real CoupledCoeffField::computeQpJacobian() {
  return 0;
}
