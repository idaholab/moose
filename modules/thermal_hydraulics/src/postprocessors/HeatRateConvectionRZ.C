#include "HeatRateConvectionRZ.h"

registerMooseObject("ThermalHydraulicsApp", HeatRateConvectionRZ);

InputParameters
HeatRateConvectionRZ::validParams()
{
  InputParameters params = HeatRateConvection::validParams();
  params += RZSymmetry::validParams();

  params.addClassDescription(
      "Integrates a cylindrical heat structure boundary convective heat flux");

  return params;
}

HeatRateConvectionRZ::HeatRateConvectionRZ(const InputParameters & parameters)
  : HeatRateConvection(parameters), RZSymmetry(this, parameters)
{
}

Real
HeatRateConvectionRZ::computeQpIntegral()
{
  const Real circumference = computeCircumference(_q_point[_qp]);
  return circumference * HeatRateConvection::computeQpIntegral();
}
