//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Kernel.h"

/**
 * Manning friction source for SWE momentum equations.
 * For variable 'hu': Sx = - g n^2 u |u| / h^(1/3); for 'hv': analogous in y.
 */
class SWEFrictionSource : public Kernel
{
public:
  static InputParameters validParams();

  SWEFrictionSource(const InputParameters & parameters);
  virtual ~SWEFrictionSource();

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  // parameters
  const Real _g;
  const Real _n_manning;
  const Real _h_eps;
  const Real _s_eps;

  // coupled variables
  const unsigned int _h_var;
  const unsigned int _hu_var;
  const unsigned int _hv_var;

  const VariableValue & _h;
  const VariableValue & _hu;
  const VariableValue & _hv;
};
