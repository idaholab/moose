//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVFluxBC.h"

class Function;

/**
 * This class computes a boundary diffusion flux by taking the difference between a provided
 * boundary value and the boundary cell centroid value. This only captures the orthogonal component
 * of the boundary diffusive flux
 */
class FVOrthogonalBoundaryDiffusion : public FVFluxBC
{
public:
  FVOrthogonalBoundaryDiffusion(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  ADReal computeQpResidual() override;

  const Function & _function;
  /// Diffusion coefficient on the element
  const ADMaterialProperty<Real> & _coeff_elem;
  /// Diffusion coefficient on the neighbor. Depending on whether this is evaluated on an interior
  /// boundary and on global element numbering, either this or \p _coeff_elem correspond to ghost
  /// element/cell evaluations
  const ADMaterialProperty<Real> & _coeff_neighbor;
  const MooseArray<ADReal> & _diff_quant_elem;
  const MooseArray<ADReal> & _diff_quant_neighbor;
};
