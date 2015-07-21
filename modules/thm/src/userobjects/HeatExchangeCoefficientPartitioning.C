#include "HeatExchangeCoefficientPartitioning.h"

template<>
InputParameters validParams<HeatExchangeCoefficientPartitioning>()
{
  InputParameters params = validParams<GeneralUserObject>();
  params.addParam<Real>("k", 5, "Steepness of the exponential function");
  params.addParam<Real>("lower", 0.001, "Lower cut-off limit");
  params.addParam<Real>("upper", 0.999, "Upper cut-off limit");

  return params;
}

HeatExchangeCoefficientPartitioning::HeatExchangeCoefficientPartitioning(const InputParameters & parameters) :
    GeneralUserObject(parameters),
    _k(getParam<Real>("k")),
    _lower(getParam<Real>("lower")),
    _upper(getParam<Real>("upper"))
{
}

HeatExchangeCoefficientPartitioning::~HeatExchangeCoefficientPartitioning()
{
}

Real
HeatExchangeCoefficientPartitioning::getPartition(Real alpha_liquid, Real dalpha_liquid_dt) const
{
  if ((alpha_liquid < _lower))
  {
    if (dalpha_liquid_dt < 0)
      return 1 - (1 - std::exp(-_k * (_lower - alpha_liquid) / _lower));
    else
      return 1 - std::exp(-_k * alpha_liquid / _lower);
  }
  else if ((alpha_liquid > _upper))
  {
    if (dalpha_liquid_dt > 0)
      return 1 - (1 - std::exp(-_k * (alpha_liquid - _upper) / _lower));
    else
      return 1 - std::exp(-_k * (1 - alpha_liquid) / _lower);
  }
  else
    return 1.;
}

Real
HeatExchangeCoefficientPartitioning::getPartitionDer(Real alpha_liquid, Real dalpha_liquid_dt, Real area) const
{
  if ((alpha_liquid < _lower))
  {
    if (dalpha_liquid_dt < 0)
      return  std::exp(-_k * (_lower - alpha_liquid) / _lower) * (_k / _lower / area);
    else
      return -std::exp(-_k * alpha_liquid / _lower) * (-_k / _lower / area);
  }
  else if ((alpha_liquid > _upper))
  {
    if (dalpha_liquid_dt > 0)
      return  std::exp(-_k * (alpha_liquid - _upper) / _lower) * (-_k / _lower / area);
    else
      return -std::exp(-_k * (1 - alpha_liquid) / _lower) * (_k / _lower / area);
  }
  else
    return 0;
}
