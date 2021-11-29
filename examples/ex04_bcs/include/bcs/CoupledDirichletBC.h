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

/// Implements a coupled Dirichlet BC where u = alpha * some_var on the boundary.
class CoupledDirichletBC : public NodalBC
{
public:
  CoupledDirichletBC(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  virtual Real computeQpResidual() override;

private:
  /// Multiplier on the boundary.
  Real _alpha;
  /// reference to a user-specifiable coupled (independent) variable
  const VariableValue & _some_var_val;
};
