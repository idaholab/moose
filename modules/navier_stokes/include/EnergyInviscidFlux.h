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

  EnergyInviscidFlux(std::string name, MooseSystem & moose_system, InputParameters parameters);
  
protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  unsigned int _u_vel_var;
  MooseArray<Real> & _u_vel;

  unsigned int _v_vel_var;
  MooseArray<Real> & _v_vel;

  unsigned int _w_vel_var;
  MooseArray<Real> & _w_vel;

  MooseArray<Real> & _pressure;
};
 
#endif
