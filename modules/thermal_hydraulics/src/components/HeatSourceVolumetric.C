#include "HeatSourceVolumetric.h"

registerMooseObject("ThermalHydraulicsApp", HeatSourceVolumetric);

InputParameters
HeatSourceVolumetric::validParams()
{
  InputParameters params = Component::validParams();
  params.addRequiredParam<std::string>("flow_channel",
                                       "Flow channel name in which to apply heat source");
  params.addRequiredParam<FunctionName>("q", "Volumetric heat source [W/m^3]");
  params.addClassDescription("Volumetric heat source applied on a flow channel");
  return params;
}

HeatSourceVolumetric::HeatSourceVolumetric(const InputParameters & parameters)
  : Component(parameters)
{
  logError("Deprecated component. Use HeatSourceVolumetric1Phase or HeatSourceVolumetric2Phase "
           "instead.");
}
