//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DerivativeFunctionMaterialBase.h"

// Forward declaration
class DiscreteNucleationMap;

/**
 * Free energy penalty contribution to force the nucleation of subresolution particles
 */
class DiscreteNucleation : public DerivativeFunctionMaterialBase
{
public:
  static InputParameters validParams();

  DiscreteNucleation(const InputParameters & params);

  virtual void computeProperties();

protected:
  unsigned int _nvar;

  /// map op_names indices to _args indices
  std::vector<unsigned int> _op_index;

  /// Target concentration values
  const std::vector<Real> _op_values;

  /// Nucleation free energy prefactor
  const Real _penalty;

  /// Match concentration exactly or use the target as a minumum or maximum value
  const unsigned int _penalty_mode;

  /// UserObject providing a map of currently active nuclei
  const DiscreteNucleationMap & _map;
};
