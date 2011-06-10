#ifndef NSENERGYINVISCIDFLUX_H
#define NSENERGYINVISCIDFLUX_H

#include "Kernel.h"

// Forward Declarations
class NSEnergyInviscidFlux;

template<>
InputParameters validParams<NSEnergyInviscidFlux>();

class NSEnergyInviscidFlux : public Kernel
{
public:

  NSEnergyInviscidFlux(const std::string & name, InputParameters parameters);
  
protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);
  
  // Aux Variables needed in the computation of the residual.
  VariableValue & _u_vel;
  VariableValue & _v_vel;
  VariableValue & _w_vel;
  VariableValue & _pressure;
  
  // Global parameter
  Real _gamma;

  // The variable numberings, assigned by Moose, for variables
  // which are "coupled" to this kernel in the sense of the Jacobian
  // structure.  That is, even if the derivative of this kernel
  // wrt rho is nonzero, it may not depend on rho explicitly.
  // Therefore we need only the index, not the variable value.
  unsigned _rho_var_number;
  unsigned _rhou_var_number;
  unsigned _rhov_var_number;
  unsigned _rhow_var_number;

  // If enthalpy is a nodal aux, we need rho to compute rho*U*H
  // in the residual.  If enthalpy is not a nodal aux, we need
  // rho to compute enthalpy.  So basically we need it either way...
  VariableValue & _rho;
  VariableValue & _enthalpy;
};
 
#endif
