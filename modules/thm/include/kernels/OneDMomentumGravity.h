#ifndef ONEDMOMENTUMGRAVITY_H
#define ONEDMOMENTUMGRAVITY_H

#include "Kernel.h"
#include "DerivativeMaterialInterfaceRelap.h"

class OneDMomentumGravity;

template <>
InputParameters validParams<OneDMomentumGravity>();

/**
 * Computes gravity term for the momentum equation
 */
class OneDMomentumGravity : public DerivativeMaterialInterfaceRelap<Kernel>
{
public:
  OneDMomentumGravity(const InputParameters & parameters);
  virtual ~OneDMomentumGravity();

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

  /// x-component of gravity
  const VariableValue & _gx;

  const unsigned int _beta_var_number;
  const unsigned int _arhoA_var_number;
};

#endif /* ONEDMOMENTUMGRAVITY_H */
