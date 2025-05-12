//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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

  virtual void initialSetup() override;

protected:
  virtual ADReal computeQpResidual() override;

  /**
   * Returns the flux input vector
   */
  virtual std::vector<ADReal> fluxInputVector() const;

  /**
   * Creates the mapping of coupled variable index to index in Euler system
   *
   * @returns the mapping of coupled variable index to index in Euler system
   */
  virtual std::map<unsigned int, unsigned int> getIndexMapping() const;

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

  /// index within the Euler system of the equation upon which this BC acts
  unsigned int _equation_index;

  /// boundary flux user object
  const ADBoundaryFluxBase & _flux;

public:
  static InputParameters validParams();
};
