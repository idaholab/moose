#pragma once

#include "Kernel.h"
#include "DerivativeMaterialInterfaceTHM.h"

class OneD3EqnMassFlux;

template <>
InputParameters validParams<OneD3EqnMassFlux>();

/**
 * Mass flux for 1-phase flow
 */
class OneD3EqnMassFlux : public DerivativeMaterialInterfaceTHM<Kernel>
{
public:
  OneD3EqnMassFlux(const InputParameters & parameters);

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

  const unsigned int _arhouA_var_number;
};
