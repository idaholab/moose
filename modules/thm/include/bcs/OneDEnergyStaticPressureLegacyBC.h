#pragma once

#include "OneDIntegratedBC.h"
#include "DerivativeMaterialInterfaceTHM.h"

/**
 *
 */
class OneDEnergyStaticPressureLegacyBC : public DerivativeMaterialInterfaceTHM<OneDIntegratedBC>
{
public:
  OneDEnergyStaticPressureLegacyBC(const InputParameters & parameters);

protected:
  virtual bool shouldApply();
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned jvar);

  bool _reversible;

  const VariableValue & _area;
  const VariableValue & _arhoA;
  const VariableValue & _arhouA;
  const VariableValue & _vel_old;

  // Variable numbers (for Jacobians)
  unsigned _arhoA_var_number;
  unsigned _arhouA_var_number;

  // Required parameters
  /// the desired input static pressure
  const Real & _p_in;

public:
  static InputParameters validParams();
};
