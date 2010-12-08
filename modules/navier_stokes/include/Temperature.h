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

  Temperature(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  unsigned int _p_var;
  VariableValue & _p;

  unsigned int _pe_var;
  VariableValue & _pe;

  unsigned int _u_vel_var;
  VariableValue & _u_vel;

  unsigned int _v_vel_var;
  VariableValue & _v_vel;

  unsigned int _w_vel_var;
  VariableValue & _w_vel;

  MaterialProperty<Real> & _c_v;
};
 
#endif
