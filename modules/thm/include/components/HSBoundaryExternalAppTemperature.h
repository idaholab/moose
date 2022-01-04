#pragma once

#include "HSBoundary.h"

/**
 * Heat structure boundary condition to set temperature values computed by an external application
 */
class HSBoundaryExternalAppTemperature : public HSBoundary
{
public:
  HSBoundaryExternalAppTemperature(const InputParameters & params);

  virtual void addVariables() override;
  virtual void addMooseObjects() override;

protected:
  /// The variable name that stores the values of temperature computed by an external application
  const VariableName & _T_ext_var_name;

public:
  static InputParameters validParams();
};
