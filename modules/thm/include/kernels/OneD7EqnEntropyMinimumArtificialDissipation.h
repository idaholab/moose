#pragma once

#include "Kernel.h"
#include "DerivativeMaterialInterfaceTHM.h"
#include "FlowModel.h"

// Forward Declarations
class OneD7EqnEntropyMinimumArtificialDissipation;

template <>
InputParameters validParams<OneD7EqnEntropyMinimumArtificialDissipation>();

/**
 * This Kernel implements the viscous regularization described in the paper
 *
 * Marc O. Delchini and Jean C. Ragusa and Ray A. Berry.
 * Viscous Regularization for the Non-equilibrium Seven-Equation Two-Phase
 *   Flow Model.
 * Journal of Scientific Computing (2016) 69: 764.
 * DOI: 10.1007/s10915-016-0217-6
 */
class OneD7EqnEntropyMinimumArtificialDissipation : public DerivativeMaterialInterfaceTHM<Kernel>
{
public:
  OneD7EqnEntropyMinimumArtificialDissipation(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  virtual Real computeQpJacobianVolumeFraction(unsigned int jvar);
  virtual Real computeQpJacobianDensity(unsigned int jvar);
  virtual Real computeQpJacobianMomentum(unsigned int jvar);
  virtual Real computeQpJacobianEnergy(unsigned int jvar);

  // Void fraction equation dissipative flux derivatives
  virtual RealVectorValue l();
  virtual RealVectorValue dl_dbeta();

  // Mass conservation equation dissipative flux derivatives
  virtual RealVectorValue f();
  virtual RealVectorValue df_dbeta();
  virtual RealVectorValue df_darhoA();

  // Momentum balance equation dissipative flux derivatives
  virtual RealVectorValue g();
  virtual RealVectorValue dg_dbeta();
  virtual RealVectorValue dg_darhoA();
  virtual RealVectorValue dg_darhouA();

  // Total energy conservation equation dissipative flux derivatives
  virtual RealVectorValue h();
  virtual RealVectorValue dh_dbeta();
  virtual RealVectorValue dh_darhoA();
  virtual RealVectorValue dh_darhouA();
  virtual RealVectorValue dh_darhoEA();

  /// which equation (mass/momentum/energy) this diffusion is acting on
  const FlowModel::EEquationType _eqn_type;

  const VariableValue & _beta;
  const VariableValue & _area;
  const VariableGradient & _grad_area;

  const MaterialProperty<Real> & _alpha;
  const MaterialProperty<Real> & _dalpha_dbeta;

  const VariableValue & _rho;
  const VariableGradient & _grad_rho;
  const VariableValue & _e;
  const VariableGradient & _grad_e;
  const VariableValue & _vel;
  const VariableGradient & _grad_vel;

  const VariableValue & _arhoA;
  const VariableGradient & _grad_arhoA;
  const VariableValue & _arhouA;
  const VariableGradient & _grad_arhouA;
  const VariableGradient & _grad_beta;

  /// Boolean for phase
  const bool _is_liquid;
  const Real _sign;
  const MaterialProperty<Real> & _visc_kappa;
  const MaterialProperty<Real> & _visc_mu;
  const MaterialProperty<Real> & _visc_beta;

  const unsigned int _beta_var_num;
  const unsigned int _arhoA_var_num;
  const unsigned int _arhouA_var_num;
  const unsigned int _arhoEA_var_num;
};
