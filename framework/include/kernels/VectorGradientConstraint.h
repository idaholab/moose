#pragma once

#include "VectorKernel.h"

/**
 * Enforces the constraint q = grad(u) for vector split variables
 * where q is a vector field and u is a scalar field
 */
class VectorGradientConstraint : public VectorKernel
{
public:
  static InputParameters validParams();
  
  VectorGradientConstraint(const InputParameters & parameters);
  
protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;
  
private:
  /// The gradient of the coupled scalar variable
  const VariableGradient & _grad_coupled;
  
  /// The coupled variable number
  unsigned int _coupled_var_num;
};