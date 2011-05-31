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

  EnergyViscousFlux(const std::string & name, InputParameters parameters);
  
protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  VariableValue & _u_vel;
  VariableValue & _v_vel;
  VariableValue & _w_vel;

  unsigned int _temp_var;
  VariableGradient & _grad_temp;
  
  VariableValue & _temp;

  MaterialProperty<RealTensorValue> & _viscous_stress_tensor;
  MaterialProperty<Real> &_thermal_conductivity;
  MaterialProperty<Real> & _dynamic_viscosity;

  // Variable numberings for off-diagonal couplings
  unsigned _rho_var_number;
  unsigned _rhou_var_number;
  unsigned _rhov_var_number;
  unsigned _rhow_var_number;
  unsigned _rhoe_var_number;

  // Variable couplings for Jacobian terms
  
  VariableValue& _rho;
  VariableValue& _rho_u;
  VariableValue& _rho_v;
  VariableValue& _rho_w;
  VariableValue& _rho_e;

  // Gradients
  VariableGradient& _grad_rho;
  VariableGradient& _grad_rho_u;
  VariableGradient& _grad_rho_v;
  VariableGradient& _grad_rho_w;
  VariableGradient& _grad_rho_e;


  // Specific heat at constant volume, treated as a single
  // constant value.
  Real _cv;
};
 
#endif
