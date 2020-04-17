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
 * This kernel calculates the residual for grain growth for a single phase,
 * poly-crystal system. A single material property gamma_asymm is used for
 * the prefactor of the cross-terms between order parameters.
 * This is the AD version of ACGrGrPoly
 */
class ADGrainGrowth : public ADGrainGrowthBase
{
public:
  static InputParameters validParams();

  ADGrainGrowth(const InputParameters & parameters);

protected:
  virtual ADReal computeDFDOP();

  const ADMaterialProperty<Real> & _gamma;
};
