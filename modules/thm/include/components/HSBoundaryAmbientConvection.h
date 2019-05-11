#pragma once

#include "BoundaryBase.h"

class HSBoundaryAmbientConvection;

template <>
InputParameters validParams<HSBoundaryAmbientConvection>();

/**
 * Boundary condition for heat transfer between heat structure and ambient environment
 */
class HSBoundaryAmbientConvection : public BoundaryBase
{
public:
  HSBoundaryAmbientConvection(const InputParameters & params);

  virtual void addMooseObjects() override;

protected:
  /// The boundary names where the boundary condition is imposed
  const std::vector<BoundaryName> & _boundary;
  /// The value of ambient temperature
  const Real & _T_ambient;
  /// The value of convective heat transfer coefficient
  const Real & _htc_ambient;
};
