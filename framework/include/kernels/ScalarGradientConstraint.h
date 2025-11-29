#pragma once

#include "Kernel.h"

/**
 * Scalar constraint kernel for a single component of gradient
 * Enforces q_x = ∂u/∂x (or q_y = ∂u/∂y, etc.)
 * 
 * This is simpler than the vector version and can be used when
 * split variables are stored as separate scalar variables
 */
class ScalarGradientConstraint : public Kernel
{
public:
  static InputParameters validParams();

  ScalarGradientConstraint(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

private:
  /// The gradient of the coupled variable
  const VariableGradient & _grad_coupled;
  
  /// Variable number of the coupled variable
  unsigned int _coupled_var_num;
  
  /// Which component of gradient (0=x, 1=y, 2=z)
  const unsigned int _component;
};