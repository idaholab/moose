//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Kernel.h"

// Forward Declarations
class IdealGasFluidProperties;

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
  static InputParameters validParams();

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
  const VariableValue & _rho_et;

  // Gradients
  const VariableGradient & _grad_rho;
  const VariableGradient & _grad_rho_u;
  const VariableGradient & _grad_rho_v;
  const VariableGradient & _grad_rho_w;
  const VariableGradient & _grad_rho_et;

  // Variable numberings
  unsigned _rho_var_number;
  unsigned _rhou_var_number;
  unsigned _rhov_var_number;
  unsigned _rhow_var_number;
  unsigned _rho_et_var_number;

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
