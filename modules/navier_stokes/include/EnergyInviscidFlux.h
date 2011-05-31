#ifndef ENERGYINVISCIDFLUX_H
#define ENERGYINVISCIDFLUX_H

#include "Kernel.h"

//Forward Declarations
class EnergyInviscidFlux;

template<>
InputParameters validParams<EnergyInviscidFlux>();

class EnergyInviscidFlux : public Kernel
{
public:

  EnergyInviscidFlux(const std::string & name, InputParameters parameters);
  
protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  // unsigned int _u_vel_var;
  VariableValue & _u_vel;

  // unsigned int _v_vel_var;
  VariableValue & _v_vel;

  // unsigned int _w_vel_var;
  VariableValue & _w_vel;

  // unsigned int _pressure_var;
  VariableValue & _pressure;
  
  // Global parameter
  Real _gamma;

  // The variable numberings, assigned by Moose, for variables
  // which are "coupled" to this kernel in the sense of the Jacobian
  // structure.  That is, even if the derivative of this kernel
  // wrt rho is nonzero, it may not depend on rho explicitly.
  // Therefore we need only the index, not the variable value.
  unsigned _p_var_number;
  unsigned _pu_var_number;
  unsigned _pv_var_number;
  unsigned _pw_var_number;

  // Unless we make enthalpy a nodal aux variable, the off-diagonal
  // derivatives will depend on _rho
  VariableValue & _p;
};
 
#endif
