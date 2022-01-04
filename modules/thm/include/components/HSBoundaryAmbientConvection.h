#pragma once

#include "HSBoundary.h"

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

  /// Ambient temperature function name
  const FunctionName & _T_ambient_fn_name;
  /// Convective heat transfer coefficient function name
  const FunctionName & _htc_ambient_fn_name;

public:
  static InputParameters validParams();
};
