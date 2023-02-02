//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "INSFEFluidIntegratedBCBase.h"
#include "Function.h"

/**
 * Specifies flow of momentum out of a boundary. This class assumes that the convection term has
 * been integrated by parts. Either a pressure or velocity boundary condition may be supplied
 * through functions; branch pressure and density may be supplied, instead of a pressure function,
 * for gravity head calculations
 */
class INSFEFluidMomentumBC : public INSFEFluidIntegratedBCBase
{
public:
  static InputParameters validParams();

  INSFEFluidMomentumBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

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
