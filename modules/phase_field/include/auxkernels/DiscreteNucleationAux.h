//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

// Forward declaration
class DiscreteNucleationMap;

/**
 * Project the DiscreteNucleationMap onto an AuxVariable field
 */
class DiscreteNucleationAux : public AuxKernel
{
public:
  static InputParameters validParams();

  DiscreteNucleationAux(const InputParameters & params);

protected:
  void precalculateValue() override;
  Real computeValue() override;

  /// UserObject providing a map of currently active nuclei
  const DiscreteNucleationMap & _map;

  /// nucleus data for the current element
  const std::vector<Real> * _nucleus;

  ///@{ Bounds for the returned values
  const Real _v0;
  const Real _v1;
  ///@}
};
