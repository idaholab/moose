#pragma once

#include "FunctionSideIntegral.h"
#include "RZSymmetry.h"

class FunctionSideIntegralRZ;

template <>
InputParameters validParams<FunctionSideIntegralRZ>();

/**
 * Integrates a function over sides for RZ geometry modeled by XY domain
 */
class FunctionSideIntegralRZ : public FunctionSideIntegral, public RZSymmetry
{
public:
  FunctionSideIntegralRZ(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral() override;
};
