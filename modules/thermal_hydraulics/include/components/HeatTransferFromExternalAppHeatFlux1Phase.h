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
 * Heat transfer specified by heat flux computed by external application going into 1-phase flow
 * channel
 */
class HeatTransferFromExternalAppHeatFlux1Phase : public HeatTransfer1PhaseBase
{
public:
  HeatTransferFromExternalAppHeatFlux1Phase(const InputParameters & parameters);

  virtual void addVariables() override;
  virtual void addMooseObjects() override;

  virtual bool isTemperatureType() const override;

public:
  static InputParameters validParams();
};
