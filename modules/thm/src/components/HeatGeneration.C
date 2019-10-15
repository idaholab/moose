#include "HeatGeneration.h"

registerMooseObject("THMApp", HeatGeneration);

template <>
InputParameters
validParams<HeatGeneration>()
{
  InputParameters params = validParams<Component>();
  params.addRequiredParam<std::string>(
      "hs", "The name of the heat structure component to put the heat source onto");
  params.addRequiredParam<std::vector<std::string>>(
      "regions", "The names of the heat structure regions where heat generation is to be applied");
  params.addParam<std::string>("power", "The component name that provides reactor power");
  params.addParam<Real>(
      "power_fraction", 1., "The fraction of reactor power that goes into the heat structure");
  params.addParam<FunctionName>("power_shape_function", "axial power shape of the fuel");
  params.addParam<VariableName>("power_density", "The name of the power density variable");

  return params;
}

HeatGeneration::HeatGeneration(const InputParameters & parameters) : Component(parameters)
{
  logError("'HeatGeneration' component is deprecated, use 'HeatSourceFromTotalPower' or "
           "'HeatSourceFromPowerDensity' insteat.");
}
