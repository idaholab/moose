#include "HeatRateRadiationRZ.h"
#include "Function.h"

registerMooseObject("THMApp", HeatRateRadiationRZ);

InputParameters
HeatRateRadiationRZ::validParams()
{
  InputParameters params = SideIntegralPostprocessor::validParams();
  params += RZSymmetry::validParams();

  params.addRequiredCoupledVar("T", "Temperature");
  params.addRequiredParam<FunctionName>("T_ambient", "Ambient temperature");
  params.addRequiredParam<Real>("emissivity", "Emissivity");
  params.addParam<FunctionName>("view_factor", "1", "View factor function");
  params.addParam<Real>("stefan_boltzmann_constant", 5.670367e-8, "Stefan-Boltzmann constant");

  params.addClassDescription(
      "Integrates a cylindrical heat structure boundary radiative heat flux");

  return params;
}

HeatRateRadiationRZ::HeatRateRadiationRZ(const InputParameters & parameters)
  : SideIntegralPostprocessor(parameters),
    RZSymmetry(this, parameters),

    _T(coupledValue("T")),
    _T_ambient(getFunction("T_ambient")),
    _emissivity(getParam<Real>("emissivity")),
    _view_factor_fn(getFunction("view_factor")),
    _sigma_stefan_boltzmann(getParam<Real>("stefan_boltzmann_constant"))
{
}

Real
HeatRateRadiationRZ::computeQpIntegral()
{
  const Real circumference = computeCircumference(_q_point[_qp]);
  const Real T4 = MathUtils::pow(_T[_qp], 4);
  const Real T4inf = MathUtils::pow(_T_ambient.value(_t, _q_point[_qp]), 4);
  return circumference * _sigma_stefan_boltzmann * _emissivity *
         _view_factor_fn.value(_t, _q_point[_qp]) * (T4inf - T4);
}
