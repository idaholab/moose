#include "MTICMult.h"

template<>
InputParameters validParams<MTICMult>()
{
  InputParameters params = validParams<InitialCondition>();
  params.addRequiredCoupledVar("var1", "Coupled variable");
  params.addRequiredParam<Real>("factor", "Some factor");

  return params;
}

MTICMult::MTICMult(const std::string & name, InputParameters parameters) :
    InitialCondition(name, parameters),
    _var1(coupledValue("var1")),
    _factor(getParam<Real>("factor"))
{
}

MTICMult::~MTICMult()
{
}

Real
MTICMult::value(const Point & /*p*/)
{
  return _var1[_qp] * _factor;
}
