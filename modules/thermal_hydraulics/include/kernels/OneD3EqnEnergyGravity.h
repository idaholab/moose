#pragma once

#include "Kernel.h"
#include "DerivativeMaterialInterfaceTHM.h"

/**
 * Computes gravity term for the energy equation in 1-phase flow
 */
class OneD3EqnEnergyGravity : public DerivativeMaterialInterfaceTHM<Kernel>
{
public:
  OneD3EqnEnergyGravity(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  const VariableValue & _A;

  const MaterialProperty<Real> & _rho;
  const MaterialProperty<Real> & _drho_darhoA;

  const MaterialProperty<Real> & _vel;
  const MaterialProperty<Real> & _dvel_darhoA;
  const MaterialProperty<Real> & _dvel_darhouA;

  /// The direction of the flow channel
  const MaterialProperty<RealVectorValue> & _dir;
  /// Gravitational acceleration vector
  const RealVectorValue & _gravity_vector;

  const unsigned int _arhoA_var_number;
  const unsigned int _arhouA_var_number;

public:
  static InputParameters validParams();
};
