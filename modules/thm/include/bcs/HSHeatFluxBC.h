#pragma once

#include "FunctionNeumannBC.h"

/**
 * Applies a specified heat flux to the side of a plate heat structure
 */
class HSHeatFluxBC : public FunctionNeumannBC
{
public:
  HSHeatFluxBC(const InputParameters & parameters);

  virtual Real computeQpResidual() override;

  /// Post-processor by which to scale boundary condition
  const PostprocessorValue & _scale_pp;

public:
  static InputParameters validParams();
};
