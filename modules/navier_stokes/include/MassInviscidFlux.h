#ifndef MASSINVISCIDFLUX_H
#define MASSINVISCIDFLUX_H

#include "Kernel.h"


//Forward Declarations
class MassInviscidFlux;

template<>
InputParameters validParams<MassInviscidFlux>();

class MassInviscidFlux : public Kernel
{
public:

  MassInviscidFlux(const std::string & name, InputParameters parameters);
  
protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  unsigned int _rhou_var_number;
  VariableValue & _rhou;

  unsigned int _rhov_var_number;
  VariableValue & _rhov;

  unsigned int _rhow_var_number;
  VariableValue & _rhow;

  unsigned int _u_vel_var;
  VariableValue & _u_vel;

  unsigned int _v_vel_var;
  VariableValue & _v_vel;

  unsigned int _w_vel_var;
  VariableValue & _w_vel;
};
 
#endif
