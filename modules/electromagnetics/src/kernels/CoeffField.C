#include "CoeffField.h"
#include "Function.h"

template <>
InputParameters
validParams<CoeffField>()
{
  InputParameters params = validParams<Reaction>();
  params.addParam<Real>("k", 1.0, "Coefficient multiplier (to be squared) for field.");
  params.addParam<FunctionName>("func", 1.0, "Function multiplier for field.");
  return params;
}

CoeffField::CoeffField(const InputParameters & parameters)
  : Reaction(parameters),

    _coefficient(getParam<Real>("k")),

    _func(getFunction("func"))

{
}

Real
CoeffField::computeQpResidual()
{
  return -std::pow(_coefficient, 2) * _func.value(_t, _q_point[_qp]) *
         Reaction::computeQpResidual();
}

Real
CoeffField::computeQpJacobian()
{
  return -std::pow(_coefficient, 2) * _func.value(_t, _q_point[_qp]) *
         Reaction::computeQpJacobian();
}
