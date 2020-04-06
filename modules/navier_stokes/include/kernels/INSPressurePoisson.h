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

/**
 * This class computes the pressure Poisson solve which is part of
 * the "split" scheme used for solving the incompressible Navier-Stokes
 * equations.
 *
 * Do not use, USE INSChorinPressurePoisson and related classes instead.
 */
class INSPressurePoisson : public Kernel
{
public:
  static InputParameters validParams();

  INSPressurePoisson(const InputParameters & parameters);

  virtual ~INSPressurePoisson() {}

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned jvar);

  // Gradients of the accleration vector, 'a'
  const VariableGradient & _grad_a1;
  const VariableGradient & _grad_a2;
  const VariableGradient & _grad_a3;

  // Variable numberings
  unsigned _a1_var_number;
  unsigned _a2_var_number;
  unsigned _a3_var_number;

  // Material properties
  const MaterialProperty<Real> & _rho;
};
