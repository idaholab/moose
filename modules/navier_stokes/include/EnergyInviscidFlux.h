#ifndef ENERGYINVISCIDFLUX_H
#define ENERGYINVISCIDFLUX_H

#include "Kernel.h"
#include "Material.h"

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

  unsigned int _u_vel_var;
  VariableValue & _u_vel;

  unsigned int _v_vel_var;
  VariableValue & _v_vel;

  unsigned int _w_vel_var;
  VariableValue & _w_vel;

  MaterialProperty<Real> & _pressure;
};
 
#endif
