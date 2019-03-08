#ifndef ONEDMOMENTUMFRICTION_H
#define ONEDMOMENTUMFRICTION_H

#include "Kernel.h"
#include "DerivativeMaterialInterfaceTHM.h"

class OneDMomentumFriction;

template <>
InputParameters validParams<OneDMomentumFriction>();

/**
 * Computes wall friction term
 *
 * See RELAP-7 Theory Manual, pg. 71, Equation (230) {eq:wall_friction_force_2phase}
 */
class OneDMomentumFriction : public DerivativeMaterialInterfaceTHM<Kernel>
{
public:
  OneDMomentumFriction(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  /// area
  const VariableValue & _A;

  /// Hydraulic diameter
  const MaterialProperty<Real> & _D_h;

  /// Volume fraction
  const MaterialProperty<Real> & _alpha;
  const MaterialProperty<Real> * const _dalpha_dbeta;

  /// Density
  const MaterialProperty<Real> & _rho;
  const MaterialProperty<Real> * const _drho_dbeta;
  const MaterialProperty<Real> & _drho_darhoA;

  /// velocity
  const MaterialProperty<Real> & _vel;
  const MaterialProperty<Real> & _dvel_darhoA;
  const MaterialProperty<Real> & _dvel_darhouA;

  /// Darcy friction factor
  const MaterialProperty<Real> & _f_D;
  const MaterialProperty<Real> * const _df_D_dbeta;
  const MaterialProperty<Real> & _df_D_darhoA;
  const MaterialProperty<Real> & _df_D_darhouA;
  const MaterialProperty<Real> & _df_D_darhoEA;

  /// two-phase multiplier
  const MaterialProperty<Real> & _mult;
  const MaterialProperty<Real> * const _dmult_dbeta;
  const MaterialProperty<Real> & _dmult_darhoA;
  const MaterialProperty<Real> & _dmult_darhouA;
  const MaterialProperty<Real> & _dmult_darhoEA;

  unsigned int _beta_var_number;
  unsigned int _arhoA_var_number;
  unsigned int _arhouA_var_number;
  unsigned int _arhoEA_var_number;
};

#endif
