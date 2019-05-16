#pragma once

#include "FunctionNeumannBC.h"

class HSHeatFluxBC;

template <>
InputParameters validParams<HSHeatFluxBC>();

/**
 * Applies a specified heat flux to the side of a plate heat structure
 */
class HSHeatFluxBC : public FunctionNeumannBC
{
public:
  HSHeatFluxBC(const InputParameters & parameters);
};
