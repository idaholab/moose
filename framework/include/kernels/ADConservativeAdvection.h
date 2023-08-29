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
 * A conservative formulation of advection, e.g. no assumptions are made about whether the velocity
 * is divergence-free and the convection term is integrated by parts
 */
class ADConservativeAdvection : public ADKernel
{
public:
  static InputParameters validParams();

  ADConservativeAdvection(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

  /// advection velocity
  const ADMaterialProperty<RealVectorValue> & _velocity;

  /// advected quantity
  const MooseArray<ADReal> & _adv_quant;
};
