#pragma once

#include "HSBoundary.h"

/**
 * Applies a specified heat flux to a heat structure boundary
 */
class HSBoundaryHeatFlux : public HSBoundary
{
public:
  HSBoundaryHeatFlux(const InputParameters & params);

  virtual void addMooseObjects() override;

protected:
  virtual void check() const override;

  /// Heat flux function name
  const FunctionName & _q_fn_name;

public:
  static InputParameters validParams();
};
