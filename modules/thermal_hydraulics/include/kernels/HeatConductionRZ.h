#pragma once

#include "HeatConduction.h"
#include "RZSymmetry.h"

/**
 * Heat conduction kernel in arbitrary RZ symmetry
 */
class HeatConductionRZ : public HeatConductionKernel, public RZSymmetry
{
public:
  HeatConductionRZ(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

public:
  static InputParameters validParams();
};
