#ifndef ONEDENTROPYMINIMUMARTIFICIALDISSIPATION_H
#define ONEDENTROPYMINIMUMARTIFICIALDISSIPATION_H

#include "Kernel.h"
#include "FlowModel.h"

class OneDEntropyMinimumArtificialDissipation;

template <>
InputParameters validParams<OneDEntropyMinimumArtificialDissipation>();

/**
 * This Kernel implements the viscous regularization described in the paper
 *
 * Marc O. Delchini and Jean C. Ragusa and Ray A. Berry.
 * Viscous Regularization for the Non-equilibrium Seven-Equation Two-Phase
 *   Flow Model.
 * Journal of Scientific Computing (2016) 69: 764.
 * DOI: 10.1007/s10915-016-0217-6
 */
class OneDEntropyMinimumArtificialDissipation : public Kernel
{
public:
  OneDEntropyMinimumArtificialDissipation(const InputParameters & parameters);

protected:
  virtual RealVectorValue f();
  virtual RealVectorValue df_drhoA();
  virtual RealVectorValue g();
  virtual RealVectorValue dg_drhoA();
  virtual RealVectorValue dg_drhouA();
  virtual RealVectorValue h();
  virtual RealVectorValue dh_drhoA();
  virtual RealVectorValue dh_drhouA();
  virtual RealVectorValue dh_drhoEA();

  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);
  virtual Real computeQpJacobianDensity(unsigned int jvar);
  virtual Real computeQpJacobianMomentum(unsigned int jvar);
  virtual Real computeQpJacobianEnergy(unsigned int jvar);

  /// which equation (mass/momentum/energy) this diffusion is acting on
  FlowModel::EEquationType _eqn_type;

  /// Coupled variables
  const VariableValue & _rhoA;
  const VariableGradient & _grad_rhoA;
  const VariableValue & _rhouA;
  const VariableGradient & _grad_rhouA;
  const VariableValue & _rhoEA;
  const VariableGradient & _grad_rhoEA;
  const VariableValue & _area;
  const VariableGradient & _grad_area;
  const VariableValue & _rho;
  const VariableGradient & _grad_rho;
  const VariableValue & _e;
  const VariableGradient & _grad_e;
  const VariableValue & _velocity;
  const VariableGradient & _grad_velocity;

  /// Material properties: viscosity coefficients for Euler Equations
  const MaterialProperty<Real> & _mu;
  const MaterialProperty<Real> & _kappa;

  // Integers for off diagonal jacobian terms
  unsigned int _rhoA_var_num;
  unsigned int _rhouA_var_num;
  unsigned int _rhoEA_var_num;
};

#endif /* ONEDENTROPYMINIMUMARTIFICIALDISSIPATION_H */
