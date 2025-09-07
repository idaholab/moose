//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADKernel.h"

/**
 * Kernel computing the weak divergence of a flux vector supplied by a material.
 * Its residual $(\nabla \psi_i, \mathbf{J})$ corresponds to $-\nabla \cdot \mathbf{J}$
 * in the strong form.
 */
class ADFluxDivergence : public ADKernel
{
public:
  static InputParameters validParams();

  ADFluxDivergence(const InputParameters & parameters);

protected:
  ADReal computeQpResidual() override;

  /// Optional base name for material properties
  const std::string _base_name;

  /// Flux vector material property
  const ADMaterialProperty<RealVectorValue> & _flux;
};
