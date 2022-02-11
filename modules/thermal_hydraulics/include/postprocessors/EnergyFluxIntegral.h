//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SideIntegralPostprocessor.h"

/**
 * Computes the boundary integral of the energy flux.
 *
 * This is used in performing conservation checks. Note that the ability to
 * satisfy the general conservation check will depend on the boundary conditions
 * used; for periodic BC, this postprocessor is not even necessary. For most
 * BC, the general conservation statement must be altered; one exception would
 * be free BC, which while not producing a well-posed problem, are useful for
 * checking conservation.
 */
class EnergyFluxIntegral : public SideIntegralPostprocessor
{
public:
  EnergyFluxIntegral(const InputParameters & parameters);

  virtual void threadJoin(const UserObject & y) override;

protected:
  virtual Real computeQpIntegral() override;

  const VariableValue & _arhouA;
  const VariableValue & _enthalpy;

public:
  static InputParameters validParams();
};
