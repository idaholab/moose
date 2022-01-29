#pragma once

#include "IntegratedBC.h"

class ConvectionHeatTransferBC : public IntegratedBC
{
public:
  ConvectionHeatTransferBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  /// Ambient temperature function
  const Function & _T_ambient_fn;
  /// Ambient heat transfer coefficient function
  const Function & _htc_ambient_fn;
  /// Post-processor by which to scale boundary condition
  const PostprocessorValue & _scale_pp;

public:
  static InputParameters validParams();
};
