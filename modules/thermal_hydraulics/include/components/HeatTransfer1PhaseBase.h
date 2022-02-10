//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "HeatTransferBase.h"

/**
 * Base class for heat transfer connections to 1-phase flow channels
 */
class HeatTransfer1PhaseBase : public HeatTransferBase
{
public:
  HeatTransfer1PhaseBase(const InputParameters & parameters);

  virtual void addMooseObjects() override;

  /**
   * Returns 1-phase wall heat transfer coefficient name
   *
   * @return The name of the 1-phase wall heat transfer coefficient variable
   */
  const MaterialPropertyName & getWallHeatTransferCoefficient1PhaseName() const;

protected:
  virtual void init() override;
  virtual void initSecondary() override;
  virtual void check() const override;

  /// 1-phase wall heat transfer coefficient name
  MaterialPropertyName _Hw_1phase_name;

public:
  static InputParameters validParams();
};
