//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef RATEPRESETBC_H
#define RATEPRESETBC_H

#include "PresetNodalBC.h"

// Forward Declarations
class RatePresetBC;
class Function;

template <>
InputParameters validParams<RatePresetBC>();

/**
 * Defines a boundary condition that imposes the value to increase 
 * according to a user-specified rate at the boundary.
 */
class RatePresetBC : public PresetNodalBC
{
public:
  RatePresetBC(const InputParameters & parameters);

protected:
  virtual Real computeQpValue() override;

  /// rate by which the variable is incremented
  const Real _rate;

  /// value of the variable at the previous time step
  const VariableValue & _u_old;
};

#endif // RATEPRESETBC_H
