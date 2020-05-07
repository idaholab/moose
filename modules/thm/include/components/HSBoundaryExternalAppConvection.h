#pragma once

#include "HSBoundary.h"

/**
 * Heat structure boundary condition to perform convective heat transfer with an external
 * application
 */
class HSBoundaryExternalAppConvection : public HSBoundary
{
public:
  HSBoundaryExternalAppConvection(const InputParameters & params);

  virtual void addVariables() override;
  virtual void addMooseObjects() override;

protected:
  virtual void check() const override;

  /// Temperature from external application
  const VariableName & _T_ext_var_name;
  /// Heat transfer coefficient from external application
  const VariableName & _htc_ext_var_name;

public:
  static InputParameters validParams();
};
