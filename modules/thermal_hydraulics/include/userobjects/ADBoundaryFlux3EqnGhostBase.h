//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADBoundaryFluxBase.h"

class ADNumericalFlux3EqnBase;

/**
 * Computes boundary fluxes for the 1-D, variable-area Euler equations using a
 * numerical flux user object and a ghost cell solution
 */
class ADBoundaryFlux3EqnGhostBase : public ADBoundaryFluxBase
{
public:
  ADBoundaryFlux3EqnGhostBase(const InputParameters & parameters);

protected:
  virtual void calcFlux(unsigned int iside,
                        dof_id_type ielem,
                        const std::vector<ADReal> & U1,
                        const RealVectorValue & normal,
                        std::vector<ADReal> & flux) const override;

  /**
   * Gets the solution vector in the ghost cell
   *
   * @param[in] U1  solution vector in the boundary (interior) cell
   *
   * @returns solution vector in the ghost cell
   */
  virtual std::vector<ADReal> getGhostCellSolution(const std::vector<ADReal> & U1) const = 0;

  /// Numerical flux user object
  const ADNumericalFlux3EqnBase & _numerical_flux;
  /// Outward normal
  const Real & _normal;

public:
  static InputParameters validParams();
};
