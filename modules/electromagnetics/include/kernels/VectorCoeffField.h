//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "VectorKernel.h"

/**
 *  Kernel representing the contribution of the PDE term $cfu$, where $c$ and
 *  $f$ are constant and function coefficients, respectively, and $u$ is a
 *  vector variable.
 */
class VectorCoeffField : public VectorKernel
{
public:
  static InputParameters validParams();

  VectorCoeffField(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

private:
  // Scalar coefficient
  Real _coefficient;

  // Function coefficient
  const Function & _func;
};
