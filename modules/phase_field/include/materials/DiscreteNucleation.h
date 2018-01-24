/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef DISCRETENUCLEATION_H
#define DISCRETENUCLEATION_H

#include "DerivativeFunctionMaterialBase.h"

// Forward declaration
class DiscreteNucleation;
class DiscreteNucleationMap;

template <>
InputParameters validParams<DiscreteNucleation>();

/**
 * Free energy penalty contribution to force the nucleation of subresolution particles
 */
class DiscreteNucleation : public DerivativeFunctionMaterialBase
{
public:
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

#endif // DISCRETENUCLEATION_H
