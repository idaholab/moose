#ifndef MOMENTUMINVISCIDFLUX_H
#define MOMENTUMINVISCIDFLUX_H

#include "Kernel.h"


//ForwardDeclarations
class MomentumInviscidFlux;

template<>
InputParameters validParams<MomentumInviscidFlux>();

class MomentumInviscidFlux : public Kernel
{
public:

  MomentumInviscidFlux(const std::string & name, InputParameters parameters);
  
protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  VariableValue & _u_vel;
  VariableValue & _v_vel;
  VariableValue & _w_vel;

  int _component;
  Real _gamma;

  VariableValue & _pressure;

  unsigned _rho_var_number;
  unsigned _rhou_var_number;
  unsigned _rhov_var_number;
  unsigned _rhow_var_number;
  unsigned _rhoe_var_number;

};
 
#endif
