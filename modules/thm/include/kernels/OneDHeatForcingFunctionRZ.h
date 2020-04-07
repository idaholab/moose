#pragma once

#include "OneDHeatForcingFunction.h"
#include "RZSymmetry.h"

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

public:
  static InputParameters validParams();
};
