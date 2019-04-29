#pragma once

#include "Kernel.h"
#include "DerivativeMaterialInterfaceTHM.h"

class OneDEnergyGravity;

template <>
InputParameters validParams<OneDEnergyGravity>();

/**
 * Computes gravity term for the energy equation
 */
class OneDEnergyGravity : public DerivativeMaterialInterfaceTHM<Kernel>
{
public:
  OneDEnergyGravity(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  const bool _has_beta;

  const VariableValue & _A;

  const MaterialProperty<Real> & _alpha;
  const MaterialProperty<Real> * const _dalpha_dbeta;

  const MaterialProperty<Real> & _rho;
  const MaterialProperty<Real> * const _drho_dbeta;
  const MaterialProperty<Real> & _drho_darhoA;

  const MaterialProperty<Real> & _vel;
  const MaterialProperty<Real> & _dvel_darhoA;
  const MaterialProperty<Real> & _dvel_darhouA;

  /// The direction of the flow channel
  const MaterialProperty<RealVectorValue> & _dir;
  /// Gravitational acceleration vector
  const RealVectorValue & _gravity_vector;

  const unsigned int _beta_var_number;
  const unsigned int _arhoA_var_number;
  const unsigned int _arhouA_var_number;
};
