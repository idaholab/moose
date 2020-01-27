#pragma once

#include "HSBoundary.h"

class HSBoundaryAmbientConvection;

template <>
InputParameters validParams<HSBoundaryAmbientConvection>();

/**
 * Boundary condition for heat transfer between heat structure and ambient environment
 */
class HSBoundaryAmbientConvection : public HSBoundary
{
public:
  HSBoundaryAmbientConvection(const InputParameters & params);

  virtual void addMooseObjects() override;

protected:
  virtual void check() const override;

  /// The value of ambient temperature
  const Real & _T_ambient;
  /// The value of convective heat transfer coefficient
  const Real & _htc_ambient;
};
