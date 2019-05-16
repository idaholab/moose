#pragma once

#include "Kernel.h"
#include "DerivativeMaterialInterfaceTHM.h"

class OneD3EqnMomentumFormLoss;
class Function;

template <>
InputParameters validParams<OneD3EqnMomentumFormLoss>();

/**
 * Computes the force per unit length due to form loss, provided a form
 * loss coefficient per unit length function
 *
 * See RELAP-7 Theory Manual, pg. 72, Equation (239) {eq:form_loss_force_2phase}
 */
class OneD3EqnMomentumFormLoss : public DerivativeMaterialInterfaceTHM<Kernel>
{
public:
  OneD3EqnMomentumFormLoss(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  /// area
  const VariableValue & _A;

  /// density
  const MaterialProperty<Real> & _rho;
  const MaterialProperty<Real> & _drho_darhoA;

  /// velocity
  const MaterialProperty<Real> & _vel;
  const MaterialProperty<Real> & _dvel_darhoA;
  const MaterialProperty<Real> & _dvel_darhouA;

  /// form loss coefficient per unit length function
  const MaterialProperty<Real> & _K_prime;

  unsigned int _arhoA_var_number;
  unsigned int _arhouA_var_number;
  unsigned int _arhoEA_var_number;
};
