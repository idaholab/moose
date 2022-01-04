#include "Sampler1DVector.h"

registerMooseObject("THMApp", Sampler1DVector);

InputParameters
Sampler1DVector::validParams()
{
  InputParameters params = Sampler1DBase<Real>::validParams();
  params.addRequiredParam<unsigned int>("index",
                                        "Index of the vector property component to sample");
  return params;
}

Sampler1DVector::Sampler1DVector(const InputParameters & parameters)
  : Sampler1DBase<std::vector<Real>>(parameters), _index(getParam<unsigned int>("index"))
{
}

Real
Sampler1DVector::getScalarFromProperty(const std::vector<Real> & property,
                                       const Point & /*curr_point*/)
{
  return property[_index];
}
