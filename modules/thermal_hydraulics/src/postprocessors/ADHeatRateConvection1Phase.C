#include "ADHeatRateConvection1Phase.h"
#include "FlowModelSinglePhase.h"

registerMooseObject("ThermalHydraulicsApp", ADHeatRateConvection1Phase);

InputParameters
ADHeatRateConvection1Phase::validParams()
{
  InputParameters params = ElementIntegralPostprocessor::validParams();

  params.addParam<MaterialPropertyName>(
      "T_wall", FlowModelSinglePhase::TEMPERATURE_WALL, "Wall temperature");
  params.addParam<MaterialPropertyName>(
      "T", FlowModelSinglePhase::TEMPERATURE, "Temperature of the fluid on the slave side");
  params.addParam<MaterialPropertyName>(
      "Hw", FlowModelSinglePhase::HEAT_TRANSFER_COEFFICIENT_WALL, "Wall heat transfer coefficient");
  params.addRequiredCoupledVar("P_hf", "heat flux perimeter");

  params.addClassDescription("Computes convective heat rate into a 1-phase flow channel");

  return params;
}

ADHeatRateConvection1Phase::ADHeatRateConvection1Phase(const InputParameters & parameters)
  : ElementIntegralPostprocessor(parameters),

    _T_wall(getADMaterialProperty<Real>("T_wall")),
    _T(getADMaterialProperty<Real>("T")),
    _Hw(getADMaterialProperty<Real>("Hw")),
    _P_hf(adCoupledValue("P_hf"))
{
}

Real
ADHeatRateConvection1Phase::computeQpIntegral()
{
  return -MetaPhysicL::raw_value(_Hw[_qp]) * MetaPhysicL::raw_value(_P_hf[_qp]) *
         (MetaPhysicL::raw_value(_T[_qp]) - MetaPhysicL::raw_value(_T_wall[_qp]));
}
