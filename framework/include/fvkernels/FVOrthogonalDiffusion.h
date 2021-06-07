//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVFluxKernel.h"

/**
 * This class computes the intercell diffusion flux by taking the difference between neighboring
 * cell centroid values. This only captures the orthogonal component of the diffusive flux
 */
class FVOrthogonalDiffusion : public FVFluxKernel
{
public:
  FVOrthogonalDiffusion(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  ADReal computeQpResidual() override;

  const ADMaterialProperty<Real> & _coeff_elem;
  const ADMaterialProperty<Real> & _coeff_neighbor;
  const MooseArray<ADReal> & _diff_quant_elem;
  const MooseArray<ADReal> & _diff_quant_neighbor;
};
