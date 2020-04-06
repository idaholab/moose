#pragma once

#include "FunctionElementIntegral.h"
#include "RZSymmetry.h"

/**
 * Integrates a function over elements for RZ geometry modeled by XY domain
 */
class FunctionElementIntegralRZ : public FunctionElementIntegral, public RZSymmetry
{
public:
  FunctionElementIntegralRZ(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral() override;

public:
  static InputParameters validParams();
};
