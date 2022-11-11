//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MDFluidIntegratedBCBase.h"
#include "Function.h"

class MDFluidMomentumBC : public MDFluidIntegratedBCBase
{
public:
  static InputParameters validParams();

  MDFluidMomentumBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

private:
  unsigned int _component;
  const MaterialProperty<Real> & _mu;
  const MaterialProperty<Real> & _mu_t;
  bool _has_pbc;
  bool _has_vbc;
  const Function * _p_fn;
  const Function * _v_fn;

  // variables for branch pressure and density
  bool _has_pbranch;
  const VariableValue & _p_branch;
  unsigned int _p_branch_var_number;
  const VariableValue & _rho_branch;
  RealVectorValue _vec_g;
  Point _branch_center;
};
