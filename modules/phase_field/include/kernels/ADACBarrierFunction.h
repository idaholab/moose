//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADGrainGrowthBase.h"

/**
 * Several kernels use a material property called mu. If mu is not a constant,
 * then this kernel will calculate the bulk AC term where mu is the derivative term.
 * It currently only takes a single value for gamma.
 **/
class ADACBarrierFunction : public ADGrainGrowthBase
{
public:
  static InputParameters validParams();

  ADACBarrierFunction(const InputParameters & parameters);

protected:
  virtual ADReal computeDFDOP();

  const ADMaterialProperty<Real> & _gamma;
  const ADMaterialProperty<Real> & _dmudvar;
};
