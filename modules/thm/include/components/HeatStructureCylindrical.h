#pragma once

#include "HeatStructureCylindricalBase.h"

/**
 * Component to model cylindrical heat structure
 */
class HeatStructureCylindrical : public HeatStructureCylindricalBase
{
public:
  HeatStructureCylindrical(const InputParameters & params);

  virtual void check() const override;

public:
  static InputParameters validParams();
};
