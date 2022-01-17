#include "HeatRateRadiationRZ.h"

registerMooseObject("ThermalHydraulicsApp", HeatRateRadiationRZ);

InputParameters
HeatRateRadiationRZ::validParams()
{
  InputParameters params = HeatRateRadiation::validParams();
  params += RZSymmetry::validParams();

  params.addClassDescription(
      "Integrates a cylindrical heat structure boundary radiative heat flux");

  return params;
}

HeatRateRadiationRZ::HeatRateRadiationRZ(const InputParameters & parameters)
  : HeatRateRadiation(parameters), RZSymmetry(this, parameters)
{
}

Real
HeatRateRadiationRZ::computeQpIntegral()
{
  const Real circumference = computeCircumference(_q_point[_qp]);
  return circumference * HeatRateRadiation::computeQpIntegral();
}
