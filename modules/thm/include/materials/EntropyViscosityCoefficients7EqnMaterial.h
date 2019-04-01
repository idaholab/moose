#ifndef ENTROPYVISCOSITYCOEFFICIENTS7EQNMATERIAL_H
#define ENTROPYVISCOSITYCOEFFICIENTS7EQNMATERIAL_H

#include "Material.h"

class NormalizationParameter;
class EntropyViscosityCoefficients7EqnMaterial;

template <>
InputParameters validParams<EntropyViscosityCoefficients7EqnMaterial>();

/**
 * This class computes the entropy viscosity coefficients for the entropy viscosity method.
 *
 * It is required to compute two coefficients: _kappa and _beta.
 * Each of these coefficients is bounded by a first order viscosity function of the eigenvalues, and
 * by an entropy viscosity coefficient function of the entropy residual.
 */
class EntropyViscosityCoefficients7EqnMaterial : public Material
{
public:
  EntropyViscosityCoefficients7EqnMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();
  // Constant:
  Real _Cmax;
  Real _Cjump;
  Real _Centropy;
  Real _Cmax_vf;
  Real _Cjump_vf;
  Real _Centropy_vf;

  // Booleans:
  bool _use_jump;
  bool _use_jump_vf;
  bool _use_first_order;
  bool _use_first_order_vf;
  bool _use_low_mach_fix;
  bool _use_parabolic_regularization;
  bool _is_liquid;

  // Density of fluid
  const VariableValue & _rho;
  const VariableValue & _rho_old;
  const VariableValue & _rho_dot;
  const VariableGradient & _grad_rho;

  // Momentum of fluid
  const VariableValue & _rhou;
  const VariableValue & _rhou_old;

  // Total energy of fluid
  const VariableValue & _rhoE;
  const VariableValue & _rhoE_old;

  // Pressure of fluid
  const VariableValue & _press;
  const VariableValue & _press_dot;
  const VariableGradient & _grad_press;

  // Void fraction
  const VariableValue & _alpha;
  const VariableValue & _alpha_dot;
  const VariableGradient & _grad_alpha;

  // Jump variables:
  const VariableValue & _jump_press;
  const VariableValue & _jump_dens;
  const VariableValue & _jump_alpha;

  const MaterialProperty<Real> & _c;
  // The material properties: viscosity coefficients
  MaterialProperty<Real> & _visc_max;
  MaterialProperty<Real> & _kappa;
  MaterialProperty<Real> & _mu;
  MaterialProperty<Real> & _beta_max;
  MaterialProperty<Real> & _beta;

  // Material properties to get: interfacial velocity and friction.
  const MaterialProperty<Real> & _vI;

  const NormalizationParameter & _norm_param;

  const PostprocessorValue & _vf_pps;

  /// The direction of the flow channel
  const MaterialProperty<RealVectorValue> & _dir;
};

#endif /* ENTROPYVISCOSITYCOEFFICIENTS7EQNMATERIAL_H */
