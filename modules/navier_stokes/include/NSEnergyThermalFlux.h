#ifndef NSENERGYTHERMALFLUX_H
#define NSENERGYTHERMALFLUX_H

#include "Kernel.h"

// ForwardDeclarations
class NSEnergyThermalFlux;

template<>
InputParameters validParams<NSEnergyThermalFlux>();


/**
 * This class is responsible for computing residuals and Jacobian
 * terms for the k * grad(T) * grad(phi) term in the Navier-Stokes
 * energy equation.
 */
class NSEnergyThermalFlux : public Kernel
{
public:

  NSEnergyThermalFlux(const std::string & name, InputParameters parameters);
  
protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  // Coupled variable values
  VariableValue& _rho;
  VariableValue& _rho_u;
  VariableValue& _rho_v;
  VariableValue& _rho_w;
  VariableValue& _rho_e;
  
  // Gradients
  VariableGradient& _grad_temp;
  VariableGradient& _grad_rho;
  VariableGradient& _grad_rho_u;
  VariableGradient& _grad_rho_v;
  VariableGradient& _grad_rho_w;
  VariableGradient& _grad_rho_e;

  // Variable numberings 
  unsigned _rho_var_number;
  unsigned _rhou_var_number;
  unsigned _rhov_var_number;
  unsigned _rhow_var_number;
  unsigned _rhoe_var_number;
  
  // Material properties
  MaterialProperty<Real> &_thermal_conductivity;

  // Parameters
  Real _cv;

private:
  // Recomputes gradient and Hessian values (only the lower triangle,
  // since its symmetric, of the temperature at the current quadrature point.
  void recalculate_gradient_and_hessian();
  
  // Returns the (i,j) entry of the hessian matrix.  If
  // j>i (upper triangle) returns (j,i) entry instead.
  Real get_hess(unsigned i, unsigned j);

  // Computes the Jacobian value (on or off-diagonal) for
  // var_number, which has been mapped to 
  // 0 = rho
  // 1 = rho*u
  // 2 = rho*v
  // 3 = rho*w
  // 4 = rho*E
  Real compute_jacobian_value(unsigned var_number);

  // dT/dU_m
  Real _dTdU[5];

  // d^2 T/ dU_m dU_n 
  Real _hessian[5][5];

  // Single vector to refer to all gradients.  We have to store
  // pointers since you can't have a vector<Foo&>.  Initialized in
  // the ctor.
  std::vector<VariableGradient*> _gradU;
};


#endif // ENERGYTHERMALFLUX_H
