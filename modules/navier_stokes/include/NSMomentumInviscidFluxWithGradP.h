#ifndef NSMOMENTUMINVISCIDFLUXWITHGRADP_H
#define NSMOMENTUMINVISCIDFLUXWITHGRADP_H

#include "Kernel.h"


// ForwardDeclarations
class NSMomentumInviscidFluxWithGradP;

template<>
InputParameters validParams<NSMomentumInviscidFluxWithGradP>();

class NSMomentumInviscidFluxWithGradP : public Kernel
{
public:

  NSMomentumInviscidFluxWithGradP(const std::string & name, InputParameters parameters);
  
protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);
  
  // Coupled variables
  VariableValue& _u_vel;
  VariableValue& _v_vel;
  VariableValue& _w_vel;
  VariableValue& _rho;
  
  // Coupled gradients
  VariableGradient& _grad_p;
  VariableGradient& _grad_rho;
  VariableGradient& _grad_rho_u;
  VariableGradient& _grad_rho_v;
  VariableGradient& _grad_rho_w;
  VariableGradient& _grad_rho_e;

  // Parameters
  int _component;
  Real _gamma;

  // Variable numbers
  unsigned _rho_var_number;
  unsigned _rhou_var_number;
  unsigned _rhov_var_number;
  unsigned _rhow_var_number;
  unsigned _rhoe_var_number;

private:
  // Recomputes gradient and Hessian values (only the lower triangle,
  // since it's symmetric, of the pressure at the current quadrature point.
  void recalculate_gradient_and_hessian();

  // Returns the (i,j) entry of the hessian matrix.  If
  // j>i (upper triangle) returns (j,i) entry instead.
  Real get_hess(unsigned i, unsigned j);

  // Computes the Jacobian contribution due to the pressure term,
  // by summing over the appropriate Hessian row.
  Real compute_pressure_jacobian_value(unsigned var_number);

  // dp/dU_m
  Real _dpdU[5];

  // d^2 p/ dU_m dU_n 
  Real _hessian[5][5];

  // Single vector to refer to all gradients.  We have to store
  // pointers since you can't have a vector<Foo&>.  Initialized in
  // the ctor.
  std::vector<VariableGradient*> _gradU;
};
 
#endif
