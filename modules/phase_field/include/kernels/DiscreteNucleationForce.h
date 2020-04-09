//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Kernel.h"

// Forward declaration
class DiscreteNucleationMap;

/**
 * Free energy penalty contribution to force the nucleation of subresolution particles
 */
class DiscreteNucleationForce : public Kernel
{
public:
  static InputParameters validParams();

  DiscreteNucleationForce(const InputParameters & params);

  void precalculateResidual() override;
  Real computeQpResidual() override;

protected:
  /// UserObject providing a map of currently active nuclei
  const DiscreteNucleationMap & _map;

  /// nucleus data for the current element
  const std::vector<Real> * _nucleus;

  ///@{ Bounds for the returned values
  const Real _v0;
  const Real _v1;
  ///@}
};
