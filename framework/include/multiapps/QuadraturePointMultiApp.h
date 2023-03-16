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
 * Automatically generates sub-App positions with elemental quadrature points of the parent mesh.
 */
class QuadraturePointMultiApp : public TransientMultiApp, public BlockRestrictable
{
public:
  static InputParameters validParams();

  QuadraturePointMultiApp(const InputParameters & parameters);

protected:
  void fillPositions() override;
};
