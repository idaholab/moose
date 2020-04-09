//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TransientMultiApp.h"
#include "BlockRestrictable.h"

/**
 * Automatically generates Sub-App positions from centroids of elements in the master mesh.
 */
class CentroidMultiApp : public TransientMultiApp, public BlockRestrictable
{
public:
  static InputParameters validParams();

  CentroidMultiApp(const InputParameters & parameters);

protected:
  /**
   * fill in _positions with the positions of the sub-aps
   */
  virtual void fillPositions() override;
};
