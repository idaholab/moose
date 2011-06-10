#ifndef NSMOMENTUMINVISCIDFLUX_H
#define NSMOMENTUMINVISCIDFLUX_H

#include "Kernel.h"


// ForwardDeclarations
class NSMomentumInviscidFlux;

template<>
InputParameters validParams<NSMomentumInviscidFlux>();

class NSMomentumInviscidFlux : public Kernel
{
public:

  NSMomentumInviscidFlux(const std::string & name, InputParameters parameters);
  
protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  // Coupled variables
  VariableValue & _u_vel;
  VariableValue & _v_vel;
  VariableValue & _w_vel;
  VariableValue & _pressure;

  // Parameters
  int _component;
  Real _gamma;

  // Variable numbers
  unsigned _rho_var_number;
  unsigned _rhou_var_number;
  unsigned _rhov_var_number;
  unsigned _rhow_var_number;
  unsigned _rhoe_var_number;
};
 
#endif
