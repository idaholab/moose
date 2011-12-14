#include "SideFluxIntegralRZ.h"

template<>
InputParameters validParams<SideFluxIntegralRZ>()
{
  InputParameters params = validParams<SideFluxIntegral>();
  return params;
}

SideFluxIntegralRZ::SideFluxIntegralRZ(const std::string & name,
                                       InputParameters parameters)
  :SideFluxIntegral(name, parameters)
{}

Real
SideFluxIntegralRZ::computeQpIntegral()
{
  return SideFluxIntegral::computeQpIntegral();
}
