#pragma once

#include "IntegratedBC.h"

/**
 * Convection BC from an external application
 */
class ExternalAppConvectionHeatTransferBC : public IntegratedBC
{
public:
  ExternalAppConvectionHeatTransferBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

  /// Temperature from external application
  const VariableValue & _T_ext;
  /// Heat transfer coefficient from external application
  const VariableValue & _htc_ext;
  /// Post-processor by which to scale boundary condition
  const PostprocessorValue & _scale_pp;

public:
  static InputParameters validParams();
};
