//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "VectorIntegratedBC.h"
#include <complex>

class VectorTransientAbsorbingBC : public VectorIntegratedBC
{
public:
  static InputParameters validParams();

  VectorTransientAbsorbingBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  const Function & _admittance;

  MooseEnum _component;

  const VectorVariableValue & _coupled_val;
  unsigned int _coupled_var_num;

  const VectorVariableValue & _u_dot;
  const VectorVariableValue & _coupled_dot;

  const VariableValue & _du_dot_du;
  const VariableValue & _coupled_dot_du;

  std::complex<double> _jay;
};
