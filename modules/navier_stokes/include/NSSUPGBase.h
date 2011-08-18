#ifndef NSSUPGBASE_H
#define NSSUPGBASE_H

#include "Kernel.h"

// Forward Declarations
class NSSUPGBase;

template<>
InputParameters validParams<NSSUPGBase>();


/**
 * This class acts as a base class for stabilization kernels.
 * This is useful because the stabilization kernels for different
 * equations share a lot of information...
 */
class NSSUPGBase : public Kernel
{
public:

  NSSUPGBase(const std::string & name, InputParameters parameters);
  
protected:
  /**
   * This kernel is not actually called at quadrature points:
   * derived classes must implement these functions
   */
  //virtual Real computeQpResidual();
  //virtual Real computeQpJacobian();
  //virtual Real computeQpOffDiagJacobian(unsigned int jvar);
  
  // Material properties
  MaterialProperty<RealTensorValue>& _viscous_stress_tensor;
  MaterialProperty<Real>& _dynamic_viscosity;
  MaterialProperty<Real>& _thermal_conductivity;
  
  // SUPG-related material properties.
  MaterialProperty<Real> & _hsupg;
  MaterialProperty<Real> & _tauc;
  MaterialProperty<Real> & _taum;
  MaterialProperty<Real> & _taue;
  MaterialProperty<std::vector<Real> > & _strong_residuals;

  // Momentum equation inviscid flux matrices
  MaterialProperty<std::vector<RealTensorValue> >& _calA;

  // "velocity column" matrices
  MaterialProperty<std::vector<RealTensorValue> >& _calC;
  
  // Energy equation inviscid flux matrices
  MaterialProperty<std::vector<std::vector<RealTensorValue> > >& _calE;

  // Coupled variable values.
  VariableValue& _rho;
  VariableValue& _rho_u;
  VariableValue& _rho_v;
  VariableValue& _rho_w;
  VariableValue& _rho_e;
  
  // "Old" (from previous timestep) coupled variable values.
  VariableValue& _rho_old;
  VariableValue& _rho_u_old;
  VariableValue& _rho_v_old;
  VariableValue& _rho_w_old;
  VariableValue& _rho_e_old;

  // Velocities are used directly in computing h_supg and tau's.
  VariableValue & _u_vel;
  VariableValue & _v_vel;
  VariableValue & _w_vel;

  // Temperature is need to compute speed of sound
  VariableValue & _temperature;

  // Enthalpy aux variable
  VariableValue& _enthalpy;

  // Gradients
  VariableGradient& _grad_rho;
  VariableGradient& _grad_rho_u;
  VariableGradient& _grad_rho_v;
  VariableGradient& _grad_rho_w;
  VariableGradient& _grad_rho_e;

  // Variable numberings
  unsigned _rho_var_number;
  unsigned _rhou_var_number;
  unsigned _rhov_var_number;
  unsigned _rhow_var_number;
  unsigned _rhoe_var_number;

  // Global parameters
  Real _gamma;
  Real _R;
  Real _cv;

  // Helper function for mapping Moose variable numberings into
  // the "canonical" numbering for the compressible NS equations.
  unsigned map_var_number(unsigned var);
};
 
#endif //  NSSUPGBASE_H
