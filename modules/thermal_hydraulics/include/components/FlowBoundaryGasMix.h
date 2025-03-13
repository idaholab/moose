//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FlowBoundary1PhaseBase.h"

/**
 * Base class for boundary components connected to FlowChannelGasMix components
 */
class FlowBoundaryGasMix : public FlowBoundary1PhaseBase
{
public:
  static InputParameters validParams();

  FlowBoundaryGasMix(const InputParameters & params);

protected:
  virtual void check() const override;

  /**
   * Creates the boundary condition objects for weak boundary conditions
   */
  virtual void addWeakBCs();
};
