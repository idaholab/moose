#ifndef ONEDMOMENTUMFRICTION_H
#define ONEDMOMENTUMFRICTION_H

#include "Kernel.h"
#include "DerivativeMaterialInterfaceRelap.h"

class OneDMomentumFriction;

template <>
InputParameters validParams<OneDMomentumFriction>();

/**
 * Computes wall friction term
 *
 * Equation (229) {eq:wall_friction_force_2phase}
 */
class OneDMomentumFriction : public DerivativeMaterialInterfaceRelap<Kernel>
{
public:
  OneDMomentumFriction(const InputParameters & parameters);
  virtual ~OneDMomentumFriction();

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  /// area
  const VariableValue & _A;

  /// velocity
  const MaterialProperty<Real> & _vel;
  const MaterialProperty<Real> & _dvel_darhoA;
  const MaterialProperty<Real> & _dvel_darhouA;

  /// drag coefficient
  const MaterialProperty<Real> & _Cw;
  const MaterialProperty<Real> * const _dCw_dbeta;
  const MaterialProperty<Real> & _dCw_drhoA;
  const MaterialProperty<Real> & _dCw_drhouA;
  const MaterialProperty<Real> & _dCw_drhoEA;

  const unsigned int _beta_var_number;
  const unsigned int _arhoA_var_number;
  const unsigned int _arhouA_var_number;
  const unsigned int _arhoEA_var_number;
};

#endif
