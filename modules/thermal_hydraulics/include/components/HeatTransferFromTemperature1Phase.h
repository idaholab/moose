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
  virtual const FEType & getFEType();

  /**
   * Adds 1-phase heat transfer kernels
   */
  void addHeatTransferKernels();

  /// The type of the wall temperature variable
  FEType _fe_type;

public:
  static InputParameters validParams();
};
