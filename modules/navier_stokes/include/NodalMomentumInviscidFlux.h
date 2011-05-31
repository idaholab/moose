#ifndef NODALMOMENTUMINVISCIDFLUX_H
#define NODALMOMENTUMINVISCIDFLUX_H

#include "Kernel.h"


//ForwardDeclarations
class NodalMomentumInviscidFlux;

template<>
InputParameters validParams<NodalMomentumInviscidFlux>();

/**
 * This kernel computes the contributions of the inviscid flux
 * vector in the momentum equation using only Nodal AuxVariables.
 * This approach is known to work better for solving compressible
 * flows of this sort.
 *
 * TODO: Figure out if we should use only nodal values for Jacobian entries as well?
 *       Figure out a better way to handle having large numbers of nodal Aux variables elegantly.
 *       Port the Jacobian changes back to MomentumInviscidFlux class
 *       Write up some latex documentation for these terms
 */
class NodalMomentumInviscidFlux : public Kernel
{
public:

  NodalMomentumInviscidFlux(const std::string & name, InputParameters parameters);
  
protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  // These are computed as nodal aux kernels in NSMomentumInviscidFluxAux
  VariableValue & _F1; // inviscid flux vector entry for x
  VariableValue & _F2; // inviscid flux vector entry for y
  VariableValue & _F3; // inviscid flux vector entry for z

  // Velocities are needed for computing Jacobian entries
  VariableValue & _u_vel;
  VariableValue & _v_vel;
  VariableValue & _w_vel;
  
  int _component;
  Real _gamma;

  // The variable numberings, assigned by Moose, for variables
  // which are "coupled" to this kernel in the sense of the Jacobian
  // structure.  That is, even though the derivative of this kernel
  // wrt rho is nonzero, it does not depend on rho explicitly.
  // Therefore we need only the index, not the variable value.
  unsigned _p_var_number;
  unsigned _pu_var_number;
  unsigned _pv_var_number;
  unsigned _pw_var_number;
  unsigned _pe_var_number;
};
 
#endif // NODALMOMENTUMINVISCIDFLUX_H
