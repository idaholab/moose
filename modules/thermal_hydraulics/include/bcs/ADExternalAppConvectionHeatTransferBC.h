#pragma once

#include "ADIntegratedBC.h"

/**
 * Convection BC from an external application
 */
class ADExternalAppConvectionHeatTransferBC : public ADIntegratedBC
{
public:
  ADExternalAppConvectionHeatTransferBC(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

  /// Temperature from external application
  const ADVariableValue & _T_ext;
  /// Heat transfer coefficient from external application
  const ADVariableValue & _htc_ext;
  /// Post-processor by which to scale boundary condition
  const PostprocessorValue & _scale_pp;

public:
  static InputParameters validParams();
};
