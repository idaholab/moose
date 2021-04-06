#include "WallTemperatureFromHS3DAux.h"

registerMooseObject("THMApp", WallTemperatureFromHS3DAux);

InputParameters
WallTemperatureFromHS3DAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription(
      "Gets the average wall temperature over each layer of a 3D heat structure.");
  params.addRequiredParam<UserObjectName>("T_wall_avg_uo",
                                          "Layered average wall temperature user object");
  return params;
}

WallTemperatureFromHS3DAux::WallTemperatureFromHS3DAux(const InputParameters & parameters)
  : AuxKernel(parameters), _T_wall_avg_uo(getUserObject<LayeredSideAverage>("T_wall_avg_uo"))
{
}

Real
WallTemperatureFromHS3DAux::computeValue()
{
  unsigned int i = _T_wall_avg_uo.getLayer(_current_elem->centroid());
  return _T_wall_avg_uo.getLayerValue(i);
}
