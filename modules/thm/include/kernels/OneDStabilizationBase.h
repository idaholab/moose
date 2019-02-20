#ifndef ONEDSTABILIZATIONBASE_H
#define ONEDSTABILIZATIONBASE_H

#include "Kernel.h"

// Forward Declarations
class OneDStabilizationBase;
class Function;

template <>
InputParameters validParams<OneDStabilizationBase>();

/**
 * This class acts as a base class for 1D equation "stabilization" kernels.
 * This currently includes both SUPG and shock-caputring stabilization terms.
 */
class OneDStabilizationBase : public Kernel
{
public:
  OneDStabilizationBase(const InputParameters & parameters);

protected:
  /**
   * This kernel is not actually called at quadrature points:
   * derived classes must implement these functions
   */
  // virtual Real computeQpResidual();
  // virtual Real computeQpJacobian();
  // virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  /**
   * Computes the SUPG contribution for the passed-in "row" i.e.
   * equation, where:
   * row  eqn
   * 0    mass
   * 1    momentum
   * 2    energy
   */
  Real supg_residual_contribution(unsigned row);

  /**
   * The Jacobian contribution due to the SUPG terms for variable m
   */
  Real supg_jacobian_contribution(unsigned k, unsigned m);

  /**
   * The strong form residuals and shock capturing parameters as material properties
   */
  const MaterialProperty<Real> & _delta_matprop;

  /// The strong residuals in a single vector
  const MaterialProperty<RealVectorValue> & _R;

  /// The advective flux matrix as a material property
  const MaterialProperty<RealTensorValue> & _A;

  /// The columns of the SUPG matrix as a material property
  const MaterialProperty<std::vector<RealVectorValue>> & _y;

  /// The gradient of the conserved variables, computed as a material property
  const MaterialProperty<RealVectorValue> & _dUdx;

  /// Derivatives of the _A matrix wrt conserved vars
  const MaterialProperty<std::vector<RealTensorValue>> & _dA;

  /// Variable numbers.  This requires coupling to the nonlinear variables
  /// so that we know their Moose variable numbering...
  unsigned _arhoA_var_number;
  unsigned _arhouA_var_number;
  unsigned _arhoEA_var_number;

  /// Maps a variable with Moose number jvar into the canonical numbering
  /// for the Euler equations.
  unsigned map_moose_var(unsigned jvar);

  /// The derivative of "udot" wrt u for each of the momentum variables.
  /// This is always 1/dt unless you are using BDF2...
  const VariableValue & _d_arhodot_du;
  const VariableValue & _d_arhoudot_du;
  const VariableValue & _d_arhoEdot_du;

  /// The gradient of the coupled velocity aux
  const VariableGradient & _grad_vel;

  /// The direction of the pipe
  const MaterialProperty<RealVectorValue> & _dir;
};

#endif //  ONEDSTABILIZATIONBASE_H
