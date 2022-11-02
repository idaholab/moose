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

class QuadraturePointMultiapp;

//template <>
//InputParameters validParams<QuadraturePointMultiapp>();

/**
 * Automatically generates Sub-App positions from the elemental quadrature points of the master mesh.
 */
class QuadraturePointMultiapp : public TransientMultiApp, public BlockRestrictable
{
public:
  static InputParameters validParams();

  QuadraturePointMultiapp(const InputParameters & parameters);

protected:
  /**
   * fill in _positions with the positions of the sub-apps
   */
  virtual void fillPositions() override;
};
