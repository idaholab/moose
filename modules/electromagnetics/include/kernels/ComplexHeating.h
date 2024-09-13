//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADKernel.h"

/**
 * This kernel implements the Laplacian operator:
 * $\nabla u \cdot \nabla \phi_i$
 */
class ComplexHeating : public ADKernel
{
public:
  static InputParameters validParams();

  ComplexHeating(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

  const ADVectorVariableValue & _E_real;
  const ADVectorVariableValue & _E_imag;
  const ADMaterialProperty<Real> & _cond;
  const Real & _scale;
};
