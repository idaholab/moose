#pragma once

#include "IntegratedBC.h"

// Forward Declarations
class ConvectionHeatTransferBC;

template <>
InputParameters validParams<ConvectionHeatTransferBC>();

class ConvectionHeatTransferBC : public IntegratedBC
{
public:
  ConvectionHeatTransferBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  /// Ambient Temperature
  const Real & _T_ambient;
  /// Heat transfer coefficient with ambient
  const Real & _htc_ambient;
};
