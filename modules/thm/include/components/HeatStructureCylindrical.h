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
  virtual Real getUnitPerimeter(const MooseEnum & side) const override;

  virtual Real getAxialOffset() const override { return _axial_offset; }

protected:
  /// Axial offset for the undisplaced mesh
  const Real & _axial_offset;
};
