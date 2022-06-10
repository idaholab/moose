#include "TurbinePressureRatioEfficiency1PhasePostprocessor.h"
#include "ADTurbinePressureRatioEfficiency1PhaseUserObject.h"

registerMooseObject("ThermalHydraulicsApp", TurbinePressureRatioEfficiency1PhasePostprocessor);

InputParameters
TurbinePressureRatioEfficiency1PhasePostprocessor::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  MooseEnum quantity("pressure_ratio efficiency");
  params.addRequiredParam<MooseEnum>("quantity", quantity, "Quantity to get");
  params.addRequiredParam<UserObjectName>("turbine_uo", "Turbine user object name");
  params.addClassDescription("Gets various quantities for a TurbinePressureRatioEfficiency1Phase");
  return params;
}

TurbinePressureRatioEfficiency1PhasePostprocessor::
    TurbinePressureRatioEfficiency1PhasePostprocessor(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),

    _quantity(getParam<MooseEnum>("quantity").getEnum<Quantity>()),
    _turbine_uo(getUserObject<ADTurbinePressureRatioEfficiency1PhaseUserObject>("turbine_uo"))
{
}

void
TurbinePressureRatioEfficiency1PhasePostprocessor::initialize()
{
}

void
TurbinePressureRatioEfficiency1PhasePostprocessor::execute()
{
}

Real
TurbinePressureRatioEfficiency1PhasePostprocessor::getValue()
{
  switch (_quantity)
  {
    case Quantity::PRESSURE_RATIO:
      return MetaPhysicL::raw_value(_turbine_uo.getPressureRatio());
      break;
    case Quantity::EFFICIENCY:
      return MetaPhysicL::raw_value(_turbine_uo.getEfficiency());
      break;
    default:
      mooseError("Invalid 'quantity' parameter.");
  }
}
