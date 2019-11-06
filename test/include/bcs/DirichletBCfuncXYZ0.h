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
#include "UsrFunc.h"

/**
 * Implements space-dependent Dirichlet BC.
 */
class DirichletBCfuncXYZ0 : public NodalBC
{
public:
  static InputParameters validParams();

  DirichletBCfuncXYZ0(const InputParameters & parameters);

  /**
   * Destructor.
   */
  virtual ~DirichletBCfuncXYZ0() {}

protected:
  virtual Real computeQpResidual();

private:
  /**
   * Parameters for the manufactured solution used.
   */
  Real _A0;
  Real _B0;
  Real _C0;
  Real _omega0;
};
