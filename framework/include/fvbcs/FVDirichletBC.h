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
 * Base class for finite volume Dirichlet boundary conditions
 */
class FVDirichletBC : public FVDirichletBCBase
{
public:
  FVDirichletBC(const InputParameters & parameters);

  static InputParameters validParams();

  ADReal boundaryValue(const FaceInfo & fi, const Moose::StateArg & state) const override;

private:
  const Real & _val;
};
