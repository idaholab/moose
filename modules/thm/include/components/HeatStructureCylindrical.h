#pragma once

#include "HeatStructureBase.h"
#include "HeatConductionModel.h"

class HeatStructureCylindrical;

template <>
InputParameters validParams<HeatStructureCylindrical>();

/**
 * Component to model cylindrical heat structure
 */
class HeatStructureCylindrical : public HeatStructureBase
{
public:
  HeatStructureCylindrical(const InputParameters & params);

  virtual void addMooseObjects() override;
  virtual Real getUnitPerimeter(const SideType & side) const override;

  /**
   * Get the inner radius of the heat structure
   *
   * @returns The inner radius of the heat structure
   */
  virtual Real getInnerRadius() const { return _inner_radius; }

protected:
  /// Inner radius of the heat structure
  const Real & _inner_radius;
};
