//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "HeatTransfer1PhaseBase.h"

/**
 * Heat transfer specified by heat flux going into 1-phase flow channel
 */
class HeatTransferFromHeatFlux1Phase : public HeatTransfer1PhaseBase
{
public:
  HeatTransferFromHeatFlux1Phase(const InputParameters & parameters);

  virtual void addMooseObjects() override;

  virtual bool isTemperatureType() const override;

protected:
  /// wall heat flux function name
  const FunctionName _q_wall_fn_name;

public:
  static InputParameters validParams();
};
