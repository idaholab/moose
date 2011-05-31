#ifndef MOMENTUMVISCOUSFLUX_H
#define MOMENTUMVISCOUSFLUX_H

#include "Kernel.h"
#include "Material.h"


//ForwardDeclarations
class MomentumViscousFlux;

template<>
InputParameters validParams<MomentumViscousFlux>();

class MomentumViscousFlux : public Kernel
{
public:

  MomentumViscousFlux(const std::string & name, InputParameters parameters);
  
protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  unsigned _component;

  MaterialProperty<RealTensorValue> & _viscous_stress_tensor;
  
  // Additional couplings needed for Jacobian entries:

  // viscosity, aka "mu", 
  MaterialProperty<Real> & _dynamic_viscosity;

  // density and its gradient
  VariableValue & _rho;
  VariableGradient & _grad_rho;
  
  // Variable numberings for off-diagonal couplings
  unsigned _rho_var_number;
  unsigned _rhou_var_number;
  unsigned _rhov_var_number;
  unsigned _rhow_var_number;

  // Variable couplings for off-diagonal Jacobian terms
  VariableValue& _rho_u;
  VariableGradient& _grad_rho_u;
  
  VariableValue& _rho_v;
  VariableGradient& _grad_rho_v;

  VariableValue& _rho_w;
  VariableGradient& _grad_rho_w;
};
 
#endif
