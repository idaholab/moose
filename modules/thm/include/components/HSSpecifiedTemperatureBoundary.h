#ifndef HSSPECIFIEDTEMPERATUREBOUNDARY_H
#define HSSPECIFIEDTEMPERATUREBOUNDARY_H

#include "BoundaryBase.h"

class HSSpecifiedTemperatureBoundary;

template <>
InputParameters validParams<HSSpecifiedTemperatureBoundary>();

/**
 * Boundary condition to set a specified value of temperature in a heat structure
 */
class HSSpecifiedTemperatureBoundary : public BoundaryBase
{
public:
  HSSpecifiedTemperatureBoundary(const InputParameters & params);

  virtual void addMooseObjects() override;

protected:
  /// The boundary names where the boundary condition is imposed
  const std::vector<BoundaryName> & _boundary;
  /// The value of temperature imposed at the boundary
  const Real & _temperature;
};

#endif /* HSSPECIFIEDTEMPERATUREBOUNDARY_H */
