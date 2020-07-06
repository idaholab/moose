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
 * Computes a heat source/sink due to convection from ambient surroundings
 */
class INSADEnergyAmbientConvection : public ADKernelValue
{
public:
  INSADEnergyAmbientConvection(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  ADReal precomputeQpResidual() override;

  const ADMaterialProperty<Real> & _temperature_ambient_convection_strong_residual;
};
