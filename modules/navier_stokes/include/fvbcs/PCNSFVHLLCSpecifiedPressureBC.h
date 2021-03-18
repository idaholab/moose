#pragma once

#include "PCNSFVHLLCBC.h"

class Function;

class PCNSFVHLLCSpecifiedPressureBC : public PCNSFVHLLCBC
{
public:
  PCNSFVHLLCSpecifiedPressureBC(const InputParameters & parameters);
  static InputParameters validParams();

protected:
  void preComputeWaveSpeed() override;

  const Function & _pressure_boundary_function;
};
