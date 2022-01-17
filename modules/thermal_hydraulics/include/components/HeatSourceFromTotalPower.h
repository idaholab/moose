#pragma once

#include "HeatSourceBase.h"

/**
 * Heat generation from total power
 */
class HeatSourceFromTotalPower : public HeatSourceBase
{
public:
  HeatSourceFromTotalPower(const InputParameters & parameters);

  virtual void addMooseObjects() override;

protected:
  virtual void init() override;
  virtual void check() const override;

  /// The name of the variable that represents total power
  VariableName _power_var_name;
  /// The fraction of the power that goes into the heat structure
  const Real & _power_fraction;
  /// true if power shape function is being used
  const bool _has_psf;
  /// The name of the power shape function
  FunctionName _power_shape_func;

public:
  static InputParameters validParams();
};
