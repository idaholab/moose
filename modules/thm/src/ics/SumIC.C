#include "SumIC.h"

registerMooseObject("THMApp", SumIC);

template <>
InputParameters
validParams<SumIC>()
{
  InputParameters params = validParams<InitialCondition>();

  params.addClassDescription("IC for sum of variables");

  params.addRequiredCoupledVar("values", "Vector of values to sum");

  return params;
}

SumIC::SumIC(const InputParameters & parameters)
  : InitialCondition(parameters), _n_values(coupledComponents("values"))
{
  for (unsigned int i = 0; i < _n_values; i++)
    _values.push_back(&coupledValue("values", i));
}

Real
SumIC::value(const Point & /*p*/)
{
  Real sum = 0;
  for (unsigned int i = 0; i < _n_values; i++)
    sum += (*(_values[i]))[_qp];

  return sum;
}
