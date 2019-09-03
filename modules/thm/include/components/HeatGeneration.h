#pragma once

#include "Component.h"

class HeatGeneration;

template <>
InputParameters validParams<HeatGeneration>();

/**
 * Adds heat generation to a heat structure
 */
class HeatGeneration : public Component
{
public:
  HeatGeneration(const InputParameters & parameters);

  virtual void addMooseObjects() override;

protected:
  virtual void init() override;
  virtual void check() const override;

  /// Names of the heat structure regions where heat generation is to be applied
  const std::vector<std::string> & _region_names;
  /// The name of the variable that represents reactor power
  VariableName _power_var_name;
  /// The fraction of the power that goes into the heat structure
  const Real & _power_fraction;
  /// true if power shape function is being used
  const bool _has_psf;
  /// The name of the power shape function
  FunctionName _power_shape_func;
  /// True is power density is given via another variable
  const bool _has_power_density;
  /// The name of the power density variable (typically an aux variable)
  const VariableName _power_density_name;
};
