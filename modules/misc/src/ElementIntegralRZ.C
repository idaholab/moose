#include "ElementIntegralRZ.h"

template<>
InputParameters validParams<ElementIntegralRZ>()
{
  InputParameters params = validParams<ElementIntegral>();
  return params;
}

ElementIntegralRZ::ElementIntegralRZ(const std::string & name, InputParameters parameters) :
    ElementIntegral(name, parameters)
{}

Real
ElementIntegralRZ::computeQpIntegral()
{
  return 2 * M_PI * _q_point[_qp](0) * ElementIntegral::computeQpIntegral();
}
