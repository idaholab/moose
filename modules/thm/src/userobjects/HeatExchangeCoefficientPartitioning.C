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

HeatExchangeCoefficientPartitioning::HeatExchangeCoefficientPartitioning(const std::string & name, InputParameters parameters) :
    GeneralUserObject(name, parameters),
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
  Real c = 1;

  if ((alpha_liquid < _lower))
  {
    if (dalpha_liquid_dt < 0)
      c = 1 - (1 - std::exp(-_k * (0.001 - alpha_liquid) / 0.001));
    else
      c = 1 - std::exp(-_k * alpha_liquid / 0.001);
  }
  else if ((alpha_liquid > _upper))
  {
    if (dalpha_liquid_dt > 0)
      c = 1 - (1 - std::exp(-_k * (alpha_liquid - 0.999) / 0.001));
    else
      c = 1 - std::exp(-_k * (1 - alpha_liquid) / 0.001);
  }
  else
    c = 1.;

  return c;
}
