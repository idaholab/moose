//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KernelValue.h"

// Forward Declarations

/**
 * TODO: This Kernel needs Documentation!!!
 */
class HHPFCRFF : public KernelValue
{
public:
  static InputParameters validParams();

  HHPFCRFF(const InputParameters & parameters);

protected:
  virtual Real precomputeQpResidual();
  virtual Real precomputeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  const Real _kernel_sign;

  const MaterialProperty<Real> & _prop;

  const bool _has_coupled_var;
  const VariableValue * const _coupled_var;
  const unsigned int _coupled_var_var;
};
