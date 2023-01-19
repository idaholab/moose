#include "ShaftConnectedCompressor1PhasePostprocessor.h"
#include "ADShaftConnectedCompressor1PhaseUserObject.h"

registerMooseObject("ThermalHydraulicsApp", ShaftConnectedCompressor1PhasePostprocessor);

InputParameters
ShaftConnectedCompressor1PhasePostprocessor::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  MooseEnum quantity("pressure_ratio efficiency rel_corrected_flow rel_corrected_speed");
  params.addRequiredParam<MooseEnum>("quantity", quantity, "Quantity to get");
  params.addRequiredParam<UserObjectName>("compressor_uo", "Compressor user object name");
  params.addClassDescription("Gets various quantities for a ShaftConnectedCompressor1Phase");
  return params;
}

ShaftConnectedCompressor1PhasePostprocessor::ShaftConnectedCompressor1PhasePostprocessor(
    const InputParameters & parameters)
  : GeneralPostprocessor(parameters),

    _quantity(getParam<MooseEnum>("quantity").getEnum<Quantity>()),
    _compressor_uo(getUserObject<ADShaftConnectedCompressor1PhaseUserObject>("compressor_uo"))
{
}

void
ShaftConnectedCompressor1PhasePostprocessor::initialize()
{
}

void
ShaftConnectedCompressor1PhasePostprocessor::execute()
{
}

Real
ShaftConnectedCompressor1PhasePostprocessor::getValue()
{
  switch (_quantity)
  {
    case Quantity::PRESSURE_RATIO:
      return MetaPhysicL::raw_value(_compressor_uo.getPressureRatio());
      break;
    case Quantity::EFFICIENCY:
      return MetaPhysicL::raw_value(_compressor_uo.getEfficiency());
      break;
    case Quantity::REL_CORRECTED_FLOW:
      return MetaPhysicL::raw_value(_compressor_uo.getRelativeCorrectedMassFlowRate());
      break;
    case Quantity::REL_CORRECTED_SPEED:
      return MetaPhysicL::raw_value(_compressor_uo.getRelativeCorrectedSpeed());
      break;
    default:
      mooseError("Invalid 'quantity' parameter.");
  }
}
