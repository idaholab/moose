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

// Forward Declaration
template <ComputeStage>
class ADCHSplitConcentration;

declareADValidParams(ADCHSplitConcentration);

/**
 * Solves Cahn-Hilliard equation using chemical potential as non-linear variable
 **/
template <ComputeStage compute_stage>
class ADCHSplitConcentration : public ADKernel<compute_stage>
{
public:
  static InputParameters validParams();

  ADCHSplitConcentration(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual();

  // Chemical potential variable gradient
  const ADVariableGradient & _grad_mu;

  // Mobility property name
  const ADMaterialProperty(Real) & _mobility;

  usingKernelMembers;
};
