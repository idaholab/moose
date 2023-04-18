//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NodalBC.h"
#include "Function.h"

/**
 * A conditional Dirichlet BC for the energy (temperature) equation
 */
class INSFEFluidEnergyDirichletBC : public NodalBC
{
public:
  static InputParameters validParams();

  INSFEFluidEnergyDirichletBC(const InputParameters & parameters);
  virtual ~INSFEFluidEnergyDirichletBC() {}

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  bool shouldApply() override;
  bool isInlet();

  RealVectorValue _out_norm;

  const VariableValue & _u_vel;
  const VariableValue & _v_vel;
  const VariableValue & _w_vel;

  Real _T_scale;
  const VariableValue & _T_scalar;

  bool _has_vbc;
  const Function * _velocity_fn;
  bool _has_T_fn;
  const Function * _T_fn;
};
