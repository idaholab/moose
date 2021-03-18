#pragma once

#include "CNSFVHLLCBC.h"

class Function;

class CNSFVHLLCSpecifiedPressureBC : public CNSFVHLLCBC
{
public:
  CNSFVHLLCSpecifiedPressureBC(const InputParameters & parameters);
  static InputParameters validParams();

protected:
  void preComputeWaveSpeed() override;

  const Function & _pressure_boundary_function;
};
