#pragma once

#include "OneDIntegratedBC.h"
#include "DerivativeMaterialInterfaceTHM.h"

/**
 *
 */
class OneDMassHRhoUBC : public DerivativeMaterialInterfaceTHM<OneDIntegratedBC>
{
public:
  OneDMassHRhoUBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  /// Specified momentum
  const Real & _rhou;
  /// Cross-sectional area
  const VariableValue & _area;

public:
  static InputParameters validParams();
};
