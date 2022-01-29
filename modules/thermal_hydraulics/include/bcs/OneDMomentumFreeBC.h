#pragma once

#include "OneDIntegratedBC.h"
#include "DerivativeMaterialInterfaceTHM.h"

/**
 * A BC for the mass equation in which nothing is specified (i.e.
 * everything is allowed to be "free".
 */
class OneDMomentumFreeBC : public DerivativeMaterialInterfaceTHM<OneDIntegratedBC>
{
public:
  OneDMomentumFreeBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned jvar);

  unsigned _arhoA_var_number;
  unsigned _arhouA_var_number;
  unsigned _arhoEA_var_number;
  const VariableValue & _vel;
  const VariableValue & _area;
  const MaterialProperty<Real> & _p;
  const MaterialProperty<Real> & _dp_darhoA;
  const MaterialProperty<Real> & _dp_darhouA;
  const MaterialProperty<Real> & _dp_darhoEA;

public:
  static InputParameters validParams();
};
