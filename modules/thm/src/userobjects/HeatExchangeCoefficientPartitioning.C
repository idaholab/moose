#include "HeatExchangeCoefficientPartitioning.h"

template <>
InputParameters
validParams<HeatExchangeCoefficientPartitioning>()
{
  InputParameters params = validParams<GeneralUserObject>();
  params.addParam<Real>("lower", 0.001, "Lower cut-off limit");
  params.addParam<Real>("upper", 0.999, "Upper cut-off limit");

  return params;
}

HeatExchangeCoefficientPartitioning::HeatExchangeCoefficientPartitioning(
    const InputParameters & parameters)
  : GeneralUserObject(parameters), _lower(getParam<Real>("lower")), _upper(getParam<Real>("upper"))
{
}

Real
HeatExchangeCoefficientPartitioning::getPartition(Real alpha_liquid, Real) const
{
  if ((alpha_liquid < _lower))
    return -alpha_liquid * alpha_liquid / _lower / _lower + 2 * alpha_liquid / _lower;
  else if ((alpha_liquid > _upper))
  {
    Real den = (_upper - 1) * (_upper - 1);
    return -alpha_liquid * alpha_liquid / den + 2 * _upper * alpha_liquid / den +
           (1 - 2 * _upper) / den;
  }
  else
    return 1.;
}

Real
HeatExchangeCoefficientPartitioning::getPartitionDer(Real alpha_liquid, Real, Real) const
{
  if ((alpha_liquid < _lower))
    return -2 * alpha_liquid / _lower / _lower + 2 / _lower;
  else if ((alpha_liquid > _upper))
  {
    Real den = (_upper - 1) * (_upper - 1);
    return -2 * alpha_liquid / den + 2 * _upper / den;
  }
  else
    return 0;
}
