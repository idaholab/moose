//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "IntegratedBC.h"

class FluidFreeSurfaceBC : public IntegratedBC
{
public:
  static InputParameters validParams();

  FluidFreeSurfaceBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

  /// Ratio of u to du/dn. alpha is the inverse of acceleration due to gravity
  const Real _alpha;
  const VariableValue & _u_dotdot;
  const VariableValue & _du_dotdot_du;
};
