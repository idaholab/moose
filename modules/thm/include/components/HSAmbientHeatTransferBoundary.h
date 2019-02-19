#ifndef HSAMBIENTHEATTRANSFERBOUNDARY_H
#define HSAMBIENTHEATTRANSFERBOUNDARY_H

#include "BoundaryBase.h"

class HSAmbientHeatTransferBoundary;

template <>
InputParameters validParams<HSAmbientHeatTransferBoundary>();

/**
 * Boundary condition for heat transfer between heat structure and ambient environment
 */
class HSAmbientHeatTransferBoundary : public BoundaryBase
{
public:
  HSAmbientHeatTransferBoundary(const InputParameters & params);

  virtual void addMooseObjects() override;

protected:
  /// The boundary names where the boundary condition is imposed
  const std::vector<BoundaryName> & _boundary;
  /// The value of ambient temperature
  const Real & _T_ambient;
  /// The value of convective heat transfer coefficient
  const Real & _htc_ambient;
};

#endif /* HSAMBIENTHEATTRANSFERBOUNDARY_H */
