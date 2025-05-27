//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "BoundaryFluxGasMixGhostBase.h"

/**
 * Wall boundary flux for FlowModelGasMix.
 */
class BoundaryFluxGasMixGhostWall : public BoundaryFluxGasMixGhostBase
{
public:
  static InputParameters validParams();

  BoundaryFluxGasMixGhostWall(const InputParameters & parameters);

protected:
  virtual std::vector<ADReal> getGhostCellSolution(const std::vector<ADReal> & U1) const override;
};
