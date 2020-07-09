//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADKernelValue.h" // Provides AD(Vector)KernelValue

/**
 * Computes an arbitrary volumetric heat source (or sink).
 */
class INSADEnergySource : public ADKernelValue
{
public:
  INSADEnergySource(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  ADReal precomputeQpResidual() override;

  const ADMaterialProperty<Real> & _temperature_source_strong_residual;
};
