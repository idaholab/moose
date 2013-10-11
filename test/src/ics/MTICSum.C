#include "MTICSum.h"

template<>
InputParameters validParams<MTICSum>()
{
  InputParameters params = validParams<InitialCondition>();
  params.addRequiredCoupledVar("var1", "First variable");
  params.addRequiredCoupledVar("var2", "Second variable");

  return params;
}

MTICSum::MTICSum(const std::string & name, InputParameters parameters) :
    InitialCondition(name, parameters),
    _var1(coupledValue("var1")),
    _var2(coupledValue("var2"))
{
}

MTICSum::~MTICSum()
{
}

Real
MTICSum::value(const Point & /*p*/)
{
  return _var1[_qp] + _var2[_qp] + 3;
}
