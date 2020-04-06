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
 * This class computes the "projection" part of the "split" method for
 * solving incompressible Navier-Stokes.  This is a time-varying equation
 * for u that is coupled to both the acceleration "a" and the pressue.
 *
 * Do not use, USE INSChorinCorrector and related classes instead.
 */
class INSProjection : public Kernel
{
public:
  static InputParameters validParams();

  INSProjection(const InputParameters & parameters);

  virtual ~INSProjection() {}

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned jvar);

  // Coupled variables
  const VariableValue & _a1;
  const VariableValue & _a2;
  const VariableValue & _a3;

  // Gradients
  const VariableGradient & _grad_p;

  // Variable numberings
  unsigned _a1_var_number;
  unsigned _a2_var_number;
  unsigned _a3_var_number;
  unsigned _p_var_number;

  // Parameters
  unsigned _component;

  // Material properties
  const MaterialProperty<Real> & _rho;
};
