#include "HeatRateConvectionRZ.h"

registerMooseObject("THMApp", HeatRateConvectionRZ);

InputParameters
HeatRateConvectionRZ::validParams()
{
  InputParameters params = SideIntegralPostprocessor::validParams();
  params += RZSymmetry::validParams();

  params.addRequiredCoupledVar("T", "Temperature");
  params.addRequiredParam<Real>("T_ambient", "Ambient Temperature");
  params.addRequiredParam<Real>("htc", "Heat transfer coefficient");

  params.addClassDescription(
      "Integrates a cylindrical heat structure boundary convective heat flux");

  return params;
}

HeatRateConvectionRZ::HeatRateConvectionRZ(const InputParameters & parameters)
  : SideIntegralPostprocessor(parameters),
    RZSymmetry(parameters),

    _T(coupledValue("T")),
    _T_ambient(getParam<Real>("T_ambient")),
    _htc(getParam<Real>("htc"))
{
}

Real
HeatRateConvectionRZ::computeQpIntegral()
{
  const Real circumference = computeCircumference(_q_point[_qp]);
  return circumference * _htc * (_T_ambient - _T[_qp]);
}
