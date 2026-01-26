//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "BoundaryFlux1PhaseBaseBC.h"

/**
 * Boundary conditions for the 1-D, 1-phase, variable-area Euler equations
 * using a boundary flux user object
 */
class ADBoundaryFlux3EqnBC : public BoundaryFlux1PhaseBaseBC
{
public:
  ADBoundaryFlux3EqnBC(const InputParameters & parameters);

protected:
  virtual std::vector<ADReal> fluxInputVector() const override;
  virtual std::map<unsigned int, unsigned int> getIndexMapping() const override;

  /// Names of the passive transport solution variables, if any [amount/m]
  const ADMaterialProperty<std::vector<Real>> & _passives_times_area;

  /// Number of passive transport variables
  const unsigned int _n_passives;

public:
  static InputParameters validParams();
};
