#ifndef ENERGYVISCOUSFLUXONLY_H
#define ENERGYVISCOUSFLUXONLY_H

#include "Kernel.h"
#include "Material.h"


// ForwardDeclarations
class EnergyViscousFluxOnly;

template<>
InputParameters validParams<EnergyViscousFluxOnly>();

/**
 * Unlike the original version of this kernel, this one
 * does not contain any thermal contributions.  Those are
 * now handled by the EnergyThermalFlux kernel instead.
 */
class EnergyViscousFluxOnly : public Kernel
{
public:

  EnergyViscousFluxOnly(const std::string & name, InputParameters parameters);
  
protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  VariableValue & _u_vel;
  VariableValue & _v_vel;
  VariableValue & _w_vel;

  MaterialProperty<RealTensorValue> & _viscous_stress_tensor;
  MaterialProperty<Real> & _dynamic_viscosity;

  // Variable numberings for off-diagonal couplings
  unsigned _rho_var_number;
  unsigned _rhou_var_number;
  unsigned _rhov_var_number;
  unsigned _rhow_var_number;

  // Variable couplings for Jacobian terms
  VariableValue& _rho;
  VariableValue& _rho_u;
  VariableValue& _rho_v;
  VariableValue& _rho_w;

  // Gradients
  VariableGradient& _grad_rho;
  VariableGradient& _grad_rho_u;
  VariableGradient& _grad_rho_v;
  VariableGradient& _grad_rho_w;
};
 
#endif // ENERGYVISCOUSFLUXONLY_H
