//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Kernel.h"

// Forward Declarations

/**
 * The penalty term may be used when Dirichlet boundary condition is applied to the entire boundary.
 */
class INSCompressibilityPenalty : public Kernel
{
public:
  static InputParameters validParams();

  INSCompressibilityPenalty(const InputParameters & parameters);

  virtual ~INSCompressibilityPenalty() {}

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned jvar);

  // penalty value.
  // smaller leads to more accurate solution, but the resulting system is also more difficult to
  // solve
  Real _penalty;
};
