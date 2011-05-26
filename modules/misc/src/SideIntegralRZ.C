#include "SideIntegralRZ.h"

template<>
InputParameters validParams<SideIntegralRZ>()
{
  InputParameters params = validParams<SideIntegral>();
  return params;
}

SideIntegralRZ::SideIntegralRZ(const std::string & name, InputParameters parameters) :
    SideIntegral(name, parameters)
{}

Real
SideIntegralRZ::computeQpIntegral()
{
  return 2 * M_PI * _q_point[_qp](0) * SideIntegral::computeQpIntegral();
}
