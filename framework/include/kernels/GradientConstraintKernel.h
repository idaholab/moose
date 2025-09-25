#pragma once

#include "Kernel.h"

/**
 * Constraint kernel that enforces q = ∇u in the nonlinear system
 * This is used for variable splitting in higher-order PDEs
 * 
 * Weak form: (q - ∇u, test) = 0
 * Where q is a vector variable and u is the original scalar variable
 */
class GradientConstraintKernel : public VectorKernel
{
public:
  static InputParameters validParams();

  GradientConstraintKernel(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

private:
  /// The coupled variable whose gradient we're constraining to
  const VariableGradient & _grad_coupled;
  
  /// Variable number of the coupled variable
  unsigned int _coupled_var_num;
  
  /// Component of the vector (0=x, 1=y, 2=z)
  const unsigned int _component;
};