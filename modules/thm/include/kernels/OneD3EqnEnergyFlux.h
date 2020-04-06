#pragma once

#include "Kernel.h"
#include "DerivativeMaterialInterfaceTHM.h"

/**
 * Energy flux for single phase flow
 */
class OneD3EqnEnergyFlux : public DerivativeMaterialInterfaceTHM<Kernel>
{
public:
  OneD3EqnEnergyFlux(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  const VariableValue & _A;

  /// The direction of the flow channel
  const MaterialProperty<RealVectorValue> & _dir;

  const MaterialProperty<Real> & _rho;
  const MaterialProperty<Real> & _drho_darhoA;

  const MaterialProperty<Real> & _vel;
  const MaterialProperty<Real> & _dvel_darhoA;
  const MaterialProperty<Real> & _dvel_darhouA;

  const MaterialProperty<Real> & _e;
  const MaterialProperty<Real> & _de_darhoA;
  const MaterialProperty<Real> & _de_darhouA;
  const MaterialProperty<Real> & _de_darhoEA;

  const MaterialProperty<Real> & _p;
  const MaterialProperty<Real> & _dp_darhoA;
  const MaterialProperty<Real> & _dp_darhouA;
  const MaterialProperty<Real> & _dp_darhoEA;

  const unsigned int _arhoA_var_number;
  const unsigned int _arhouA_var_number;

public:
  static InputParameters validParams();
};
