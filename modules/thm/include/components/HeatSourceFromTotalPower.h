#pragma once

#include "Component.h"

class HeatSourceFromTotalPower;

template <>
InputParameters validParams<HeatSourceFromTotalPower>();

/**
 * Heat generation from total power
 */
class HeatSourceFromTotalPower : public Component
{
public:
  HeatSourceFromTotalPower(const InputParameters & parameters);

  virtual void addMooseObjects() override;

protected:
  virtual void init() override;
  virtual void check() const override;

  /// Names of the heat structure regions where heat generation is to be applied
  const std::vector<std::string> & _region_names;
  /// The name of the variable that represents total power
  VariableName _power_var_name;
  /// The fraction of the power that goes into the heat structure
  const Real & _power_fraction;
  /// true if power shape function is being used
  const bool _has_psf;
  /// The name of the power shape function
  FunctionName _power_shape_func;
};
