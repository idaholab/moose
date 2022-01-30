#pragma once

#include "ADIntegratedBC.h"

class ADConvectionHeatTransferBC : public ADIntegratedBC
{
public:
  ADConvectionHeatTransferBC(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

  /// Ambient temperature function
  const Function & _T_ambient_fn;
  /// Ambient heat transfer coefficient function
  const Function & _htc_ambient_fn;
  /// Post-processor by which to scale boundary condition
  const PostprocessorValue & _scale_pp;

public:
  static InputParameters validParams();
};
