#ifndef NSVISCOUSFLUXBASE_H
#define NSVISCOUSFLUXBASE_H

#include "Kernel.h"
#include "Material.h"


// ForwardDeclarations
class NSViscousFluxBase;

template<>
InputParameters validParams<NSViscousFluxBase>();


/**
 * This class acts as a base class to both the:
 * .) NSMomentumViscousFlux
 * .) NSEnergyViscousFlux
 * kernels.  Common information, such as the viscous
 * stress tensor and its derivatives are computed here
 * and can be shared by both derived classes.
 */
class NSViscousFluxBase : public Kernel
{
public:

  NSViscousFluxBase(const std::string & name, InputParameters parameters);
  
protected:
  /**
   * This kernel is not actually called at quadrature points:
   * derived classes must implement these functions
   */
  //virtual Real computeQpResidual();
  //virtual Real computeQpJacobian();
  //virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  // Material properties
  MaterialProperty<RealTensorValue> & _viscous_stress_tensor;
  MaterialProperty<Real> & _dynamic_viscosity;
  
  // Coupled variable values.
  VariableValue& _rho;
  VariableValue& _rho_u;
  VariableValue& _rho_v;
  VariableValue& _rho_w;
  VariableValue& _rho_e; // None of the child classes need this value, we only use it to get the correct variable number

  // Gradients
  VariableGradient& _grad_rho;
  VariableGradient& _grad_rho_u;
  VariableGradient& _grad_rho_v;
  VariableGradient& _grad_rho_w;

  // Variable numberings
  unsigned _rho_var_number;
  unsigned _rhou_var_number;
  unsigned _rhov_var_number;
  unsigned _rhow_var_number;
  unsigned _rhoe_var_number;
};
 
#endif //  NSVISCOUSFLUXBASE_H
