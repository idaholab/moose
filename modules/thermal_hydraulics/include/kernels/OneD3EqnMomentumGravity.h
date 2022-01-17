#pragma once

#include "Kernel.h"
#include "DerivativeMaterialInterfaceTHM.h"

/**
 * Computes gravity term for the momentum equation for 1-phase flow
 */
class OneD3EqnMomentumGravity : public DerivativeMaterialInterfaceTHM<Kernel>
{
public:
  OneD3EqnMomentumGravity(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  const VariableValue & _A;

  const MaterialProperty<Real> & _rho;
  const MaterialProperty<Real> & _drho_darhoA;

  /// The direction of the flow channel
  const MaterialProperty<RealVectorValue> & _dir;
  /// Gravitational acceleration vector
  const RealVectorValue & _gravity_vector;

  const unsigned int _arhoA_var_number;

public:
  static InputParameters validParams();
};
