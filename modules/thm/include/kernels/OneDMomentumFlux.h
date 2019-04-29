#pragma once

#include "Kernel.h"
#include "DerivativeMaterialInterfaceTHM.h"

class OneDMomentumFlux;

template <>
InputParameters validParams<OneDMomentumFlux>();

/**
 * Momentum flux
 */
class OneDMomentumFlux : public DerivativeMaterialInterfaceTHM<Kernel>
{
public:
  OneDMomentumFlux(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  const bool _has_beta;

  const VariableValue & _A;

  /// The direction of the flow channel
  const MaterialProperty<RealVectorValue> & _dir;

  const MaterialProperty<Real> & _alpha;
  const MaterialProperty<Real> * const _dalpha_dbeta;

  const MaterialProperty<Real> & _rho;
  const MaterialProperty<Real> * const _drho_dbeta;
  const MaterialProperty<Real> & _drho_darhoA;

  const MaterialProperty<Real> & _vel;
  const MaterialProperty<Real> & _dvel_darhoA;
  const MaterialProperty<Real> & _dvel_darhouA;

  const MaterialProperty<Real> & _p;
  const MaterialProperty<Real> * const _dp_dbeta;
  const MaterialProperty<Real> & _dp_darhoA;
  const MaterialProperty<Real> & _dp_darhouA;
  const MaterialProperty<Real> & _dp_darhoEA;

  const unsigned int _beta_var_number;
  const unsigned int _arhoA_var_number;
  const unsigned int _arhouA_var_number;
  const unsigned int _arhoEA_var_number;
};
