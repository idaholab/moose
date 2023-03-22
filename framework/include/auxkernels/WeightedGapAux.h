//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MortarNodalAuxKernel.h"

/**
 * Compute a weighted gap based on a mortar discretization
 */
class WeightedGapAux : public MortarNodalAuxKernel
{
public:
  static InputParameters validParams();

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  WeightedGapAux(const InputParameters & parameters);

protected:
  Real computeValue() override;

  void computeQpProperties();

  void computeQpIProperties();

  /// The weighted gap
  Real _weighted_gap;

  /// The gap vector at the current quadrature point, used when we are not interpolating the normal
  /// vector, multipled by JxW
  RealVectorValue _qp_gap_nodal;
};
