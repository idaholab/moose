//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// Moose Includes
#include "DiracKernel.h"

/**
 * A DiracKernel with both on and off-diagonal Jacobian contributions
 * to test the off-diagonal contribution capability for Dirac kernels.
 */
class NonlinearSource : public DiracKernel
{
public:
  static InputParameters validParams();

  NonlinearSource(const InputParameters & parameters);

  virtual void addPoints();
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

protected:
  // A coupled variable this kernel depends on
  const VariableValue & _coupled_var;
  unsigned _coupled_var_num;

  // A constant factor which controls the strength of the source.
  Real _scale_factor;
  Point _p;
};
