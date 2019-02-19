#include "Sampler1DReal.h"

registerMooseObject("THMApp", Sampler1DReal);

template <>
InputParameters
validParams<Sampler1DReal>()
{
  InputParameters params = validParams<Sampler1DBase<Real>>();
  return params;
}

Sampler1DReal::Sampler1DReal(const InputParameters & parameters) : Sampler1DBase<Real>(parameters)
{
}

Real
Sampler1DReal::getScalarFromProperty(const Real & property, const Point & /*curr_point*/)
{
  return property;
}
