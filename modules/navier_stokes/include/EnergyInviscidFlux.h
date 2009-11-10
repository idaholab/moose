#ifndef ENERGYINVISCIDFLUX_H
#define ENERGYINVISCIDFLUX_H

#include "Kernel.h"
#include "Material.h"


//ForwardDeclarations
class EnergyInviscidFlux;

template<>
InputParameters valid_params<EnergyInviscidFlux>();

class EnergyInviscidFlux : public Kernel
{
public:

  EnergyInviscidFlux(std::string name,
                  InputParameters parameters,
                  std::string var_name,
                  std::vector<std::string> coupled_to=std::vector<std::string>(0),
                     std::vector<std::string> coupled_as=std::vector<std::string>(0));
  

  virtual void subdomainSetup();
  
protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  unsigned int _u_vel_var;
  std::vector<Real> & _u_vel;

  unsigned int _v_vel_var;
  std::vector<Real> & _v_vel;

  unsigned int _w_vel_var;
  std::vector<Real> & _w_vel;

  std::vector<Real> * _pressure;
};
 
#endif
