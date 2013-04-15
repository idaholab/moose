#include "VectorMagnitudeAux.h"

template<>
InputParameters validParams<VectorMagnitudeAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredCoupledVar("x", "x-component of the vector");
  params.addCoupledVar("y", "y-component of the vector");
  params.addCoupledVar("z", "z-component of the vector");

  return params;
}

VectorMagnitudeAux::VectorMagnitudeAux(const std::string & name, InputParameters parameters) :
    AuxKernel(name, parameters),
    _x(coupledValue("x")),
    _y(_dim >= 2 ? coupledValue("y") : _zero),
    _z(_dim >= 3 ? coupledValue("z") : _zero)
{
}

VectorMagnitudeAux::~VectorMagnitudeAux()
{
}

Real
VectorMagnitudeAux::computeValue()
{
  return std::sqrt((_x[_qp] * _x[_qp]) + (_y[_qp] * _y[_qp]) + (_z[_qp] * _z[_qp]));
}
