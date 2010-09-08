#ifndef ENERGYVISCOUSFLUX_H
#define ENERGYVISCOUSFLUX_H

#include "Kernel.h"
#include "Material.h"


//ForwardDeclarations
class EnergyViscousFlux;

template<>
InputParameters validParams<EnergyViscousFlux>();

class EnergyViscousFlux : public Kernel
{
public:

  EnergyViscousFlux(const std::string & name, MooseSystem & moose_system, InputParameters parameters);
  
protected:
  virtual Real computeQpResidual();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  unsigned int _u_vel_var;
  VariableValue & _u_vel;

  unsigned int _v_vel_var;
  VariableValue & _v_vel;

  unsigned int _w_vel_var;
  VariableValue & _w_vel;

  unsigned int _temp_var;
  VariableGradient & _grad_temp;

  MaterialProperty<RealTensorValue> & _viscous_stress_tensor;
  MaterialProperty<Real> &_thermal_conductivity;
};
 
#endif
