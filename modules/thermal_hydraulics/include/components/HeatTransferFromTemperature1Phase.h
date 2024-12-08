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
 * Base class for heat transfer connections from temperature for 1-phase flow
 */
class HeatTransferFromTemperature1Phase : public HeatTransfer1PhaseBase
{
public:
  HeatTransferFromTemperature1Phase(const InputParameters & parameters);

  virtual void addVariables() override;
  virtual void addMooseObjects() override;

  virtual bool isTemperatureType() const override;

protected:
  /// Get the FE type for wall temperature variable
  virtual const libMesh::FEType & getFEType();

  /**
   * Adds 1-phase heat transfer kernels
   */
  void addHeatTransferKernels();

  /// The type of the wall temperature variable
  libMesh::FEType _fe_type;

public:
  static InputParameters validParams();
};
