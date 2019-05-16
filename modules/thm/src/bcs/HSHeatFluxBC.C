#include "HSHeatFluxBC.h"

registerMooseObject("THMApp", HSHeatFluxBC);

template <>
InputParameters
validParams<HSHeatFluxBC>()
{
  InputParameters params = validParams<FunctionNeumannBC>();

  params.addClassDescription("Applies a specified heat flux to the side of a plate heat structure");

  return params;
}

HSHeatFluxBC::HSHeatFluxBC(const InputParameters & parameters) : FunctionNeumannBC(parameters) {}
