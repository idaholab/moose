#pragma once

#include "Component.h"

class HeatSourceFromPowerDensity;

template <>
InputParameters validParams<HeatSourceFromPowerDensity>();

/**
 * Heat source from power density
 */
class HeatSourceFromPowerDensity : public Component
{
public:
  HeatSourceFromPowerDensity(const InputParameters & parameters);

  virtual void addMooseObjects() override;

protected:
  virtual void check() const override;

  /// Names of the heat structure regions where heat generation is to be applied
  const std::vector<std::string> & _region_names;
  /// The name of the power density variable (typically an aux variable)
  const VariableName _power_density_name;
};
