#include "SumAux.h"

registerMooseObject("THMApp", SumAux);

template <>
InputParameters
validParams<SumAux>()
{
  InputParameters params = validParams<AuxKernel>();

  params.addClassDescription("Sum of aux variables");

  params.addRequiredCoupledVar("values", "Vector of values to sum");

  return params;
}

SumAux::SumAux(const InputParameters & parameters)
  : AuxKernel(parameters), _n_values(coupledComponents("values"))
{
  for (unsigned int i = 0; i < _n_values; i++)
    _values.push_back(&coupledValue("values", i));
}

Real
SumAux::computeValue()
{
  Real sum = 0;
  for (unsigned int i = 0; i < _n_values; i++)
    sum += (*(_values[i]))[_qp];

  return sum;
}
