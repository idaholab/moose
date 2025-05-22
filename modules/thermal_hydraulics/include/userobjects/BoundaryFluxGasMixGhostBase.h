//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADBoundaryFluxBase.h"

class NumericalFluxGasMixBase;

/**
 * Base class for boundary fluxes for FlowModelGasMix using ghost cells.
 */
class BoundaryFluxGasMixGhostBase : public ADBoundaryFluxBase
{
public:
  static InputParameters validParams();

  BoundaryFluxGasMixGhostBase(const InputParameters & parameters);

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
  const NumericalFluxGasMixBase & _numerical_flux;
  /// Outward normal
  const Real & _normal;
};
