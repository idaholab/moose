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
