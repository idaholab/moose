#include "Sampler1DReal.h"

registerMooseObject("THMApp", Sampler1DReal);

InputParameters
Sampler1DReal::validParams()
{
  InputParameters params = Sampler1DBase<Real>::validParams();
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
