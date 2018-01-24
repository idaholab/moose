/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COUPLEDBEKINETIC_H
#define COUPLEDBEKINETIC_H

#include "Kernel.h"

// Forward Declarations
class CoupledBEKinetic;

template <>
InputParameters validParams<CoupledBEKinetic>();

/**
 * Define the Kernel for a CoupledBEKinetic operator that looks like:
 * delta (weight * v) / delta t.
 */
class CoupledBEKinetic : public Kernel
{
public:
  CoupledBEKinetic(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;

private:
  /// Material property of porosity.
  const MaterialProperty<Real> & _porosity;

  /// Weight of the kinetic mineral concentration in the total primary species concentration.
  const std::vector<Real> _weight;

  /// Coupled kinetic mineral concentrations.
  std::vector<const VariableValue *> _vals;

  /// Coupled old values of kinetic mineral concentrations.
  std::vector<const VariableValue *> _vals_old;
};

#endif // COUPLEDBEKINETIC_H
