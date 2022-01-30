#include "SideFluxIntegralRZ.h"

registerMooseObject("ThermalHydraulicsApp", SideFluxIntegralRZ);

InputParameters
SideFluxIntegralRZ::validParams()
{
  InputParameters params = SideFluxIntegral::validParams();
  params += RZSymmetry::validParams();

  params.addClassDescription("Integrates a diffusive flux over a boundary of a 2D RZ domain.");

  return params;
}

SideFluxIntegralRZ::SideFluxIntegralRZ(const InputParameters & parameters)
  : SideFluxIntegral(parameters), RZSymmetry(this, parameters)
{
}

Real
SideFluxIntegralRZ::computeQpIntegral()
{
  const Real circumference = computeCircumference(_q_point[_qp]);
  return circumference * SideFluxIntegral::computeQpIntegral();
}
