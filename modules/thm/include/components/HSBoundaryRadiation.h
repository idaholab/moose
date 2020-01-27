#pragma once

#include "HSBoundary.h"

class HSBoundaryRadiation;

template <>
InputParameters validParams<HSBoundaryRadiation>();

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
};
