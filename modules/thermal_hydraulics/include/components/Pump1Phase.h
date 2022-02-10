//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "VolumeJunction1Phase.h"

/**
 * Pump between 1-phase flow channels that has a non-zero volume
 */
class Pump1Phase : public VolumeJunction1Phase
{
public:
  Pump1Phase(const InputParameters & params);

protected:
  virtual void buildVolumeJunctionUserObject() override;

  /// Pump head [m]
  const Real & _head;

public:
  static InputParameters validParams();
};
