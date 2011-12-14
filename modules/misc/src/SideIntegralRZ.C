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
  return SideIntegral::computeQpIntegral();
}
