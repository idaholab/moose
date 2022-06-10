#pragma once

#include "GeneralPostprocessor.h"

class ADTurbinePressureRatioEfficiency1PhaseUserObject;

/**
 * Gets various quantities for a TurbinePressureRatioEfficiency1Phase
 */
class TurbinePressureRatioEfficiency1PhasePostprocessor : public GeneralPostprocessor
{
public:
  TurbinePressureRatioEfficiency1PhasePostprocessor(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual Real getValue() override;

protected:
  /// Quantity type
  enum class Quantity
  {
    PRESSURE_RATIO,
    EFFICIENCY
  };
  /// Quantity to get
  const Quantity _quantity;

  /// 1-phase pressure ratio efficiency turbine user object
  const ADTurbinePressureRatioEfficiency1PhaseUserObject & _turbine_uo;

public:
  static InputParameters validParams();
};
