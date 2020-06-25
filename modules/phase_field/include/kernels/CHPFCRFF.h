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

/**
 * This kernel calculates the main portion of the cahn-hilliard residual for the
 * RFF form of the phase field crystal model
 */
class CHPFCRFF : public Kernel
{
public:
  static InputParameters validParams();

  CHPFCRFF(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

private:
  const MaterialProperty<Real> & _M;
  const bool _has_MJac;
  const MaterialProperty<Real> * const _DM;

  const MooseEnum _log_approach;
  const Real _tol;

  const unsigned int _num_L;
  std::vector<unsigned int> _vals_var;
  std::vector<const VariableGradient *> _grad_vals;

  const unsigned int _n_exp_terms;
  const Real _a;
  const Real _b;
  const Real _c;
};
