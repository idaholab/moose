#pragma once

#include "HSBoundary.h"

class HSBoundaryHeatFlux;

template <>
InputParameters validParams<HSBoundaryHeatFlux>();

/**
 * Applies a specified heat flux to a heat structure boundary
 */
class HSBoundaryHeatFlux : public HSBoundary
{
public:
  HSBoundaryHeatFlux(const InputParameters & params);

  virtual void addMooseObjects() override;

protected:
  /// Heat flux function name
  const FunctionName & _q_fn_name;
};
