#pragma once

#include "HSBoundary.h"

class HSBoundarySpecifiedTemperature;

template <>
InputParameters validParams<HSBoundarySpecifiedTemperature>();

/**
 * Boundary condition to set a specified value of temperature in a heat structure
 */
class HSBoundarySpecifiedTemperature : public HSBoundary
{
public:
  HSBoundarySpecifiedTemperature(const InputParameters & params);

  virtual void addMooseObjects() override;

protected:
  /// The value of temperature imposed at the boundary
  const Real & _temperature;
};
