//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADIntegratedBC.h"

class Function;

class ADHDGAdvectionDirichletBC : public ADIntegratedBC
{
public:
  static InputParameters validParams();

  ADHDGAdvectionDirichletBC(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

  /// The velocity
  const ADMaterialProperty<RealVectorValue> * const _velocity;
  const Function * const _velocity_func;

  const Real _coeff;

  const Function * const _func;
};
