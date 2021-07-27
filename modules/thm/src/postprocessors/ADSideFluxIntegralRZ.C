#include "ADSideFluxIntegralRZ.h"

registerMooseObject("THMApp", ADSideFluxIntegralRZ);

InputParameters
ADSideFluxIntegralRZ::validParams()
{
  InputParameters params = ADSideFluxIntegral::validParams();
  params += RZSymmetry::validParams();

  params.addClassDescription("Integrates a diffusive flux over a boundary of a 2D RZ domain.");

  return params;
}

ADSideFluxIntegralRZ::ADSideFluxIntegralRZ(const InputParameters & parameters)
  : ADSideFluxIntegral(parameters), RZSymmetry(parameters)
{
}

Real
ADSideFluxIntegralRZ::computeQpIntegral()
{
  const Real circumference = computeCircumference(_q_point[_qp]);
  return circumference * ADSideFluxIntegral::computeQpIntegral();
}
