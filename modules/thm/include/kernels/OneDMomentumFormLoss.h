#ifndef ONEDMOMENTUMFORMLOSS_H
#define ONEDMOMENTUMFORMLOSS_H

#include "Kernel.h"
#include "DerivativeMaterialInterfaceTHM.h"

class OneDMomentumFormLoss;
class Function;

template <>
InputParameters validParams<OneDMomentumFormLoss>();

/**
 * Computes the force per unit length due to form loss, provided a form
 * loss coefficient per unit length function
 *
 * See RELAP-7 Theory Manual, pg. 72, Equation (239) {eq:form_loss_force_2phase}
 */
class OneDMomentumFormLoss : public DerivativeMaterialInterfaceTHM<Kernel>
{
public:
  OneDMomentumFormLoss(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  /// area
  const VariableValue & _A;

  /// volume fraction
  const MaterialProperty<Real> & _alpha;
  const MaterialProperty<Real> * const _dalpha_dbeta;

  /// density
  const MaterialProperty<Real> & _rho;
  const MaterialProperty<Real> * const _drho_dbeta;
  const MaterialProperty<Real> & _drho_darhoA;

  /// velocity
  const MaterialProperty<Real> & _vel;
  const MaterialProperty<Real> & _dvel_darhoA;
  const MaterialProperty<Real> & _dvel_darhouA;

  /// two-phase multiplier
  const MaterialProperty<Real> & _mult;
  const MaterialProperty<Real> * const _dmult_dbeta;
  const MaterialProperty<Real> & _dmult_darhoA;
  const MaterialProperty<Real> & _dmult_darhouA;
  const MaterialProperty<Real> & _dmult_darhoEA;

  /// form loss coefficient per unit length function
  Function & _K_prime;

  unsigned int _beta_var_number;
  unsigned int _arhoA_var_number;
  unsigned int _arhouA_var_number;
  unsigned int _arhoEA_var_number;
};

#endif
