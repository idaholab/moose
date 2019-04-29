#pragma once

#include "FlowBoundary.h"

class Inlet;

template <>
InputParameters validParams<Inlet>();

/**
 * Inlet component remains for backwards compatibility
 */
class Inlet : public FlowBoundary
{
public:
  Inlet(const InputParameters & params);

protected:
  enum BoundaryType
  {
    BC_TYPE_STATIC_PT = 1,
    BC_TYPE_STAGNATION_PT = 2,
    BC_TYPE_DENSITY_VELOCITY = 3,
    BC_TYPE_H_RHOU = 4,
    BC_TYPE_MASS_FLOW_RATE = 5
  };

  virtual void check() const override;
};
