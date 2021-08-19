#include "HeatRateConvectionRZ.h"
#include "Function.h"

registerMooseObject("THMApp", HeatRateConvectionRZ);

InputParameters
HeatRateConvectionRZ::validParams()
{
  InputParameters params = SideIntegralPostprocessor::validParams();
  params += RZSymmetry::validParams();

  params.addRequiredCoupledVar("T", "Temperature");
  params.addRequiredParam<FunctionName>("T_ambient", "Ambient temperature function");
  params.addRequiredParam<FunctionName>("htc", "Ambient heat transfer coefficient function");

  params.addClassDescription(
      "Integrates a cylindrical heat structure boundary convective heat flux");

  return params;
}

HeatRateConvectionRZ::HeatRateConvectionRZ(const InputParameters & parameters)
  : SideIntegralPostprocessor(parameters),
    RZSymmetry(this, parameters),

    _T(coupledValue("T")),
    _T_ambient_fn(getFunction("T_ambient")),
    _htc_ambient_fn(getFunction("htc"))
{
}

Real
HeatRateConvectionRZ::computeQpIntegral()
{
  const Real circumference = computeCircumference(_q_point[_qp]);
  return circumference * _htc_ambient_fn.value(_t, _q_point[_qp]) *
         (_T_ambient_fn.value(_t, _q_point[_qp]) - _T[_qp]);
}
