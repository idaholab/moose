#pragma once

#include "HSBoundary.h"

/**
 * Boundary condition to set a specified value of temperature in a heat structure
 */
class HSBoundarySpecifiedTemperature : public HSBoundary
{
public:
  HSBoundarySpecifiedTemperature(const InputParameters & params);

  virtual void addMooseObjects() override;

protected:
  /// The function prescribing the temperature at the boundary
  const FunctionName & _T_func;

public:
  static InputParameters validParams();
};
