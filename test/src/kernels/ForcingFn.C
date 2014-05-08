#include "ForcingFn.h"

template<>
InputParameters validParams<ForcingFn>()
{
  return validParams<Kernel>();
}


ForcingFn::ForcingFn(const std::string & name, InputParameters parameters) :
    Kernel(name, parameters)
{
}

Real
ForcingFn::funcValue()
{
//  Point pt = _qrule->get_points()[_qp];
  Point pt = _q_point[_qp];

//  return (pt(0)*pt(0) + pt(1)*pt(1));
  if (_var.number() == 0)
    return (pt(0)*pt(0) + pt(1)*pt(1));
  else
    return -4;
}

Real
ForcingFn::computeQpResidual()
{
  return -funcValue() * _test[_i][_qp];
}
