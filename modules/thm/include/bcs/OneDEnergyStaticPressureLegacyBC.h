#pragma once

#include "OneDIntegratedBC.h"
#include "DerivativeMaterialInterfaceTHM.h"

// Forward Declarations
class OneDEnergyStaticPressureLegacyBC;

template <>
InputParameters validParams<OneDEnergyStaticPressureLegacyBC>();

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

  const MaterialProperty<Real> & _alpha;
  const MaterialProperty<Real> * const _dalpha_dbeta;

  const VariableValue & _area;
  const VariableValue & _arhoA;
  const VariableValue & _arhouA;
  const VariableValue & _vel_old;

  // Variable numbers (for Jacobians)
  unsigned _arhoA_var_number;
  unsigned _arhouA_var_number;
  unsigned _beta_var_num;

  // Required parameters
  /// the desired input static pressure
  const Real & _p_in;
};
