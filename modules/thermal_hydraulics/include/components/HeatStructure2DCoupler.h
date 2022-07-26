//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "HeatStructure2DCouplerBase.h"

/**
 * Couples boundaries of two 2D heat structures via a heat transfer coefficient
 */
class HeatStructure2DCoupler : public HeatStructure2DCouplerBase
{
public:
  HeatStructure2DCoupler(const InputParameters & parameters);

  virtual void addMooseObjects() override;

protected:
  virtual void check() const override;

public:
  static InputParameters validParams();
};
