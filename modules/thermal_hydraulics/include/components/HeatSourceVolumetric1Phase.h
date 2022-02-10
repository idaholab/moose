//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Component.h"

/**
 * Volumetric heat source applied on a 1-phase flow channel
 */
class HeatSourceVolumetric1Phase : public Component
{
public:
  HeatSourceVolumetric1Phase(const InputParameters & parameters);

  virtual void check() const override;
  virtual void addMooseObjects() override;

public:
  static InputParameters validParams();
};
