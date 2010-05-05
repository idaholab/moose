#ifndef TEMPERATURE_H
#define TEMPERATURE_H

#include "Kernel.h"
#include "Material.h"

//Forward Declarations
class Temperature;

template<>
InputParameters validParams<Temperature>();

class Temperature : public Kernel
{
public:

  Temperature(std::string name, MooseSystem & moose_system, InputParameters parameters);
  
  virtual void subdomainSetup();
  
protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  unsigned int _p_var;
  std::vector<Real> & _p;

  unsigned int _pe_var;
  std::vector<Real> & _pe;

  unsigned int _u_vel_var;
  std::vector<Real> & _u_vel;

  unsigned int _v_vel_var;
  std::vector<Real> & _v_vel;

  unsigned int _w_vel_var;
  std::vector<Real> & _w_vel;

  Real * _c_v;
};
 
#endif
