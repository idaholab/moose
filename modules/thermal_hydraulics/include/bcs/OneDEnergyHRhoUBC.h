#pragma once

#include "OneDIntegratedBC.h"
#include "DerivativeMaterialInterfaceTHM.h"

/**
 *
 */
class OneDEnergyHRhoUBC : public DerivativeMaterialInterfaceTHM<OneDIntegratedBC>
{
public:
  OneDEnergyHRhoUBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  /// Specified enthalpy
  const Real & _H;
  /// Specified momentum
  const Real & _rhou;
  /// Cross-sectional area
  const VariableValue & _area;

public:
  static InputParameters validParams();
};
