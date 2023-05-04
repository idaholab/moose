//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "HSBoundary.h"

/**
 * Radiative heat transfer boundary condition for heat structure
 */
class HSBoundaryRadiation : public HSBoundary
{
public:
  HSBoundaryRadiation(const InputParameters & params);

  virtual void addMooseObjects() override;

  static InputParameters validParams();
};
