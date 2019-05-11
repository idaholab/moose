#pragma once

#include "BoundaryBase.h"
#include "HSBoundaryInterface.h"

class HSBoundarySpecifiedTemperature;

template <>
InputParameters validParams<HSBoundarySpecifiedTemperature>();

/**
 * Boundary condition to set a specified value of temperature in a heat structure
 */
class HSBoundarySpecifiedTemperature : public BoundaryBase, public HSBoundaryInterface
{
public:
  HSBoundarySpecifiedTemperature(const InputParameters & params);

  virtual void check() const override;
  virtual void addMooseObjects() override;

protected:
  /// The value of temperature imposed at the boundary
  const Real & _temperature;
};
