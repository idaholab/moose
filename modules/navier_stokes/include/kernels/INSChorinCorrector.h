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
 * This class computes the "Chorin" Corrector equation in fully-discrete
 * (both time and space) form.
 */
class INSChorinCorrector : public Kernel
{
public:
  static InputParameters validParams();

  INSChorinCorrector(const InputParameters & parameters);

  virtual ~INSChorinCorrector() {}

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned jvar);

  // "Star" velocity components
  const VariableValue & _u_vel_star;
  const VariableValue & _v_vel_star;
  const VariableValue & _w_vel_star;

  // Pressure gradients
  const VariableGradient & _grad_p;

  // Variable numberings
  unsigned _u_vel_star_var_number;
  unsigned _v_vel_star_var_number;
  unsigned _w_vel_star_var_number;
  unsigned _p_var_number;

  // Parameters
  unsigned _component;

  // Material properties
  const MaterialProperty<Real> & _rho;
};
