#pragma once

#include "ADFunctionNeumannBC.h"

/**
 * Applies a specified heat flux to the side of a plate heat structure
 */
class ADHSHeatFluxBC : public ADFunctionNeumannBC
{
public:
  ADHSHeatFluxBC(const InputParameters & parameters);

  virtual ADReal computeQpResidual() override;

  /// Post-processor by which to scale boundary condition
  const PostprocessorValue & _scale_pp;

public:
  static InputParameters validParams();
};
