//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVDirichletBCBase.h"

/**
 * Applies a Dirichlet boundary condition with a value prescribed by a function
 */
class FVFunctionDirichletBC : public FVDirichletBCBase
{
public:
  FVFunctionDirichletBC(const InputParameters & parameters);

  static InputParameters validParams();

  ADReal boundaryValue(const FaceInfo & fi) const override;

private:
  const Function & _function;
};
