//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "HSBoundary.h"

/**
 * Boundary condition for heat transfer between heat structure and ambient environment
 */
class HSBoundaryAmbientConvection : public HSBoundary
{
public:
  HSBoundaryAmbientConvection(const InputParameters & params);

  virtual void addMooseObjects() override;

  static InputParameters validParams();
};
