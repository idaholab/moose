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

/* A conditional Dirichlet BC for the energy (temperature) equation  */
class MDFluidEnergyDirichletBC : public NodalBC
{
public:
  static InputParameters validParams();

  MDFluidEnergyDirichletBC(const InputParameters & parameters);
  virtual ~MDFluidEnergyDirichletBC() {}

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  bool shouldApply();
  bool isInlet();

  RealVectorValue _out_norm;

  const VariableValue & _u_vel;
  const VariableValue & _v_vel;
  const VariableValue & _w_vel;

  bool _has_vbc;
  bool _has_Tbc;
  const Function * _velocity_fn;
  const Function * _temperature_fn;
};
