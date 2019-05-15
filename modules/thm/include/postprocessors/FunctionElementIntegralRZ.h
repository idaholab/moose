#pragma once

#include "FunctionElementIntegral.h"
#include "RZSymmetry.h"

class FunctionElementIntegralRZ;

template <>
InputParameters validParams<FunctionElementIntegralRZ>();

/**
 * Integrates a function over elements for RZ geometry modeled by XY domain
 */
class FunctionElementIntegralRZ : public FunctionElementIntegral, public RZSymmetry
{
public:
  FunctionElementIntegralRZ(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral() override;
};
