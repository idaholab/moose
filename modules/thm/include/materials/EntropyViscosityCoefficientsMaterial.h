#pragma once

#include "Material.h"
#include "DerivativeMaterialInterfaceTHM.h"

class NormalizationParameter;
class EntropyViscosityCoefficientsMaterial;

template <>
InputParameters validParams<EntropyViscosityCoefficientsMaterial>();

/**
 * This class computes the entropy viscosity coefficients for the entropy viscosity method.
 *
 * It is required to compute three coefficients: _mu, _kappa and _kappa_void.
 * Each of these coefficients is bounded by a first order viscosity function of the eigenvalues, and
 * by an entropy viscosity coefficient function of the characteristic equations.
 */
class EntropyViscosityCoefficientsMaterial : public DerivativeMaterialInterfaceTHM<Material>
{
public:
  EntropyViscosityCoefficientsMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  Real computeViscosity(Real h, Real residual, Real jump, Real rho, Real norm);
  Real computeJump(Real press, Real press_grad, Real dens, Real dens_grad, Real norm);

  // Constants:
  Real _Cmax;
  Real _Cjump;
  Real _Centropy;

  // Booleans:
  bool _use_first_order;
  bool _use_low_mach_fix;
  bool _use_parabolic_regularization;
  bool _use_jump;

  // Density of fluid
  const VariableValue & _rho;
  const VariableValue & _rho_dot;
  const VariableGradient & _grad_rho;

  // Momentum of fluid
  const VariableValue & _rhou;

  // Total energy of fluid
  const VariableValue & _rhoE;

  // Pressure of fluid
  const VariableValue & _press_dot;
  const VariableGradient & _grad_press;

  // Jump variables:
  const VariableValue & _jump_press;
  const VariableValue & _jump_dens;

  const MaterialProperty<Real> & _c;
  // The material properties: viscosity coefficients
  MaterialProperty<Real> & _kappa;
  MaterialProperty<Real> & _mu;
  MaterialProperty<Real> & _dmu_drhoA;
  MaterialProperty<Real> & _dmu_drhouA;
  MaterialProperty<Real> & _dmu_drhoEA;
  MaterialProperty<Real> & _visc_max;
  MaterialProperty<Real> & _res;

  /// The direction of the flow channel
  const MaterialProperty<RealVectorValue> & _dir;

  // EOS object
  const NormalizationParameter & _norm_param;
};
