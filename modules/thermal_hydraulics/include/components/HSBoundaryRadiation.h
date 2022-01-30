#pragma once

#include "HSBoundary.h"

/**
 * Radiative heat transfer boundary condition for heat structure
 */
class HSBoundaryRadiation : public HSBoundary
{
public:
  HSBoundaryRadiation(const InputParameters & params);

  virtual void addMooseObjects() override;

protected:
  virtual void check() const override;

public:
  static InputParameters validParams();
};
