#ifndef MOMENTUMINVISCIDFLUX_H
#define MOMENTUMINVISCIDFLUX_H

#include "Kernel.h"
#include "Material.h"


//ForwardDeclarations
class MomentumInviscidFlux;

template<>
InputParameters validParams<MomentumInviscidFlux>();

class MomentumInviscidFlux : public Kernel
{
public:

  MomentumInviscidFlux(std::string name, MooseSystem & moose_system, InputParameters parameters);
  
protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  unsigned int _u_vel_var;
  VariableValue & _u_vel;

  unsigned int _v_vel_var;
  VariableValue & _v_vel;

  unsigned int _w_vel_var;
  VariableValue & _w_vel;

  int _component;

  MaterialProperty<Real> & _pressure;
};
 
#endif
