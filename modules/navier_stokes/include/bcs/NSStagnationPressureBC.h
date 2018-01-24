/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef NSSTAGNATIONPRESSUREBC_H
#define NSSTAGNATIONPRESSUREBC_H

#include "NSStagnationBC.h"

// Forward Declarations
class NSStagnationPressureBC;

// Specialization required of all user-level Moose objects
template <>
InputParameters validParams<NSStagnationPressureBC>();

/**
 * This Dirichlet condition imposes the condition p_0 = p_0_desired,
 * where p_0 is the stagnation pressure, defined as:
 * p_0 = p * (1 + (gam-1)/2 * M^2)^(gam/(gam-1))
 */
class NSStagnationPressureBC : public NSStagnationBC
{
public:
  NSStagnationPressureBC(const InputParameters & parameters);

protected:
  // NodalBC's can (currently) only specialize the computeQpResidual function,
  // the computeQpJacobian() function automatically assembles a "1" onto the main
  // diagonal for this DoF.
  virtual Real computeQpResidual();

  // Coupled variables
  const VariableValue & _pressure;

  // Required paramters
  const Real _desired_stagnation_pressure;
};

#endif // NSSTAGNATIONPRESSUREBC_H
