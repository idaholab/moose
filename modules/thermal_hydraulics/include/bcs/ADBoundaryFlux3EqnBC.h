//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADOneDIntegratedBC.h"
#include "ADBoundaryFluxBase.h"

/**
 * Boundary conditions for the 1-D, 1-phase, variable-area Euler equations
 * using a boundary flux user object
 */
class ADBoundaryFlux3EqnBC : public ADOneDIntegratedBC
{
public:
  ADBoundaryFlux3EqnBC(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

  /**
   * Creates the mapping of coupled variable index to index in Euler system
   *
   * @returns the mapping of coupled variable index to index in Euler system
   */
  std::map<unsigned int, unsigned int> getIndexMapping() const;

  /// Cross-sectional area, linear
  const ADVariableValue & _A_linear;

  // conservative variables
  const ADMaterialProperty<Real> & _rhoA;
  const ADMaterialProperty<Real> & _rhouA;
  const ADMaterialProperty<Real> & _rhoEA;

  // coupled variable indices
  const unsigned int _rhoA_var;
  const unsigned int _rhouA_var;
  const unsigned int _rhoEA_var;

  /// map of coupled variable index to equations variable index convention
  const std::map<unsigned int, unsigned int> _jmap;

  /// index within the Euler system of the equation upon which this BC acts
  const unsigned int _equation_index;

  /// boundary flux user object
  const ADBoundaryFluxBase & _flux;

public:
  static InputParameters validParams();
};
