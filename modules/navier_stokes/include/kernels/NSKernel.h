/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef NSKERNEL_H
#define NSKERNEL_H

#include "Kernel.h"

// Forward Declarations
class NSKernel;
class IdealGasFluidProperties;

template <>
InputParameters validParams<NSKernel>();

/**
 * This class couples together all the variables
 * for the compressible Navier-Stokes equations to
 * allow them to be used in derived Kernel
 * classes.  This prevents duplication of lines
 * of code between e.g. the momentum and energy
 * equations, since they have a lot in common.
 */
class NSKernel : public Kernel
{
public:
  NSKernel(const InputParameters & parameters);

protected:
  // Coupled variables
  const VariableValue & _u_vel;
  const VariableValue & _v_vel;
  const VariableValue & _w_vel;

  const VariableValue & _rho;
  const VariableValue & _rho_u;
  const VariableValue & _rho_v;
  const VariableValue & _rho_w;
  const VariableValue & _rho_E;

  // Gradients
  const VariableGradient & _grad_rho;
  const VariableGradient & _grad_rho_u;
  const VariableGradient & _grad_rho_v;
  const VariableGradient & _grad_rho_w;
  const VariableGradient & _grad_rho_E;

  // Variable numberings
  unsigned _rho_var_number;
  unsigned _rhou_var_number;
  unsigned _rhov_var_number;
  unsigned _rhow_var_number;
  unsigned _rhoE_var_number;

  // Integrated BC can use Mat. properties...
  const MaterialProperty<Real> & _dynamic_viscosity;
  const MaterialProperty<RealTensorValue> & _viscous_stress_tensor; // Includes _dynamic_viscosity

  // Fluid properties
  const IdealGasFluidProperties & _fp;

  /**
   * Helper functions for mapping Moose variable numberings into
   * the "canonical" numbering for the compressible NS equations.
   */
  bool isNSVariable(unsigned var);
  unsigned mapVarNumber(unsigned var);
};

#endif // NSKERNEL_H
