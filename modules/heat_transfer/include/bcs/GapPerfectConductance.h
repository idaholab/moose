//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "IntegratedBC.h"

/**
 * Boundary condition enforcing perfect gap conductance across gap through the use of a
 * penalty parameter.
 */
class GapPerfectConductance : public IntegratedBC
{
public:
  static InputParameters validParams();
  GapPerfectConductance(const InputParameters & parameters);

  virtual ~GapPerfectConductance() {}

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

  /// AuxVariable holding the gap_distance
  const VariableValue & _gap_distance;

  /// AuxVariable holding the temperature on the secondary surface
  const VariableValue & _gap_temp;

  /// Penatly applied to the difference between the temperature on both sides of the gap
  const Real _penalty;
};
