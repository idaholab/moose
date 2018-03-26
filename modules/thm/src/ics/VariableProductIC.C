#include "VariableProductIC.h"

registerMooseObject("RELAP7App", VariableProductIC);

template <>
InputParameters
validParams<VariableProductIC>()
{
  InputParameters params = validParams<InitialCondition>();
  params.addRequiredCoupledVar("values", "The values being multiplied");
  return params;
}

VariableProductIC::VariableProductIC(const InputParameters & parameters)
  : InitialCondition(parameters), _n(coupledComponents("values"))
{
  _values.resize(_n);
  for (unsigned int i = 0; i < _n; i++)
    _values[i] = &coupledValue("values", i);
}

Real
VariableProductIC::value(const Point & /*p*/)
{
  Real val = 1;
  for (unsigned int i = 0; i < _n; i++)
    val *= (*_values[i])[_qp];
  return val;
}
