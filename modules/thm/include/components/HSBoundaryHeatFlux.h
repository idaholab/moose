#pragma once

#include "BoundaryBase.h"
#include "HSBoundaryInterface.h"

class HSBoundaryHeatFlux;

template <>
InputParameters validParams<HSBoundaryHeatFlux>();

/**
 * Applies a specified heat flux to a heat structure boundary
 */
class HSBoundaryHeatFlux : public BoundaryBase, public HSBoundaryInterface
{
public:
  HSBoundaryHeatFlux(const InputParameters & params);

  virtual void check() const override;
  virtual void addMooseObjects() override;

protected:
  /// Heat flux function name
  const FunctionName & _q_fn_name;
};
