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

/**
 * This class computes the mass equation residual and Jacobian
 * contributions (the latter using automatic differentiation) for the incompressible Navier-Stokes
 * equations.
 */
class INSADConservativeMassWeakDiriBC : public ADIntegratedBC
{
public:
  static InputParameters validParams();

  INSADConservativeMassWeakDiriBC(const InputParameters & parameters);

protected:
  ADReal computeQpResidual() override;

  /// The velocity
  const Function & _diri_vel;
};
