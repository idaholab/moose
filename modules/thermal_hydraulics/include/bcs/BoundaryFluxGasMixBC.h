//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADBoundaryFlux3EqnBC.h"

/**
 * Boundary conditions for a FlowChannelGasMix using a boundary flux object.
 */
class BoundaryFluxGasMixBC : public ADBoundaryFlux3EqnBC
{
public:
  static InputParameters validParams();

  BoundaryFluxGasMixBC(const InputParameters & parameters);

protected:
  virtual std::vector<ADReal> fluxInputVector() const override;
  virtual std::map<unsigned int, unsigned int> getIndexMapping() const override;

  /// x*rho*A
  const ADMaterialProperty<Real> & _xirhoA;
  /// Coupled variable index for x*rho*A
  const unsigned int _xirhoA_var;
};
