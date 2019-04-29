#pragma once

#include "OneDHeatForcingFunction.h"
#include "RZSymmetry.h"

class OneDHeatForcingFunctionRZ;

template <>
InputParameters validParams<OneDHeatForcingFunctionRZ>();

/**
 * Forcing function used in the heat conduction equation in arbitrary RZ symmetry
 */
class OneDHeatForcingFunctionRZ : public OneDHeatForcingFunction, public RZSymmetry
{
public:
  OneDHeatForcingFunctionRZ(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
};
