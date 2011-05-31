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


//  unsigned int _pressure_var;
//  VariableValue & _pressure;
};
 
#endif // NODALMOMENTUMINVISCIDFLUX_H
