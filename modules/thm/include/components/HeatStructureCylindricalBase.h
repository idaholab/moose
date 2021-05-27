#pragma once

#include "HeatStructureBase.h"

/**
 * Base class for cylindrical heat structure components
 */
class HeatStructureCylindricalBase : public HeatStructureBase
{
public:
  HeatStructureCylindricalBase(const InputParameters & params);

  virtual void setupMesh() override;
  virtual void addMooseObjects() override;
  virtual Real getUnitPerimeter(const HeatStructureSideType & side) const override;

  /**
   * Get the inner radius of the heat structure
   *
   * @returns The inner radius of the heat structure
   */
  virtual Real getInnerRadius() const { return _inner_radius; }

protected:
  /// Inner radius of the heat structure
  Real _inner_radius;

public:
  static InputParameters validParams();
};
