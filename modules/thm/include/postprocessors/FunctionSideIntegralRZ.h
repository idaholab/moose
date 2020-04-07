#pragma once

#include "FunctionSideIntegral.h"
#include "RZSymmetry.h"

/**
 * Integrates a function over sides for RZ geometry modeled by XY domain
 */
class FunctionSideIntegralRZ : public FunctionSideIntegral, public RZSymmetry
{
public:
  FunctionSideIntegralRZ(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral() override;

public:
  static InputParameters validParams();
};
