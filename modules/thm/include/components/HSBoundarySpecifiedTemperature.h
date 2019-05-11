#pragma once

#include "BoundaryBase.h"

class HSBoundarySpecifiedTemperature;

template <>
InputParameters validParams<HSBoundarySpecifiedTemperature>();

/**
 * Boundary condition to set a specified value of temperature in a heat structure
 */
class HSBoundarySpecifiedTemperature : public BoundaryBase
{
public:
  HSBoundarySpecifiedTemperature(const InputParameters & params);

  virtual void addMooseObjects() override;

protected:
  /// The boundary names where the boundary condition is imposed
  const std::vector<BoundaryName> & _boundary;
  /// The value of temperature imposed at the boundary
  const Real & _temperature;
};
