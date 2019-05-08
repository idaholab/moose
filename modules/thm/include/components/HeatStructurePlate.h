#pragma once

#include "HeatStructureBase.h"
#include "HeatConductionModel.h"

class HeatStructurePlate;

template <>
InputParameters validParams<HeatStructurePlate>();

/**
 * Component to model plate heat structure
 */
class HeatStructurePlate : public HeatStructureBase
{
public:
  HeatStructurePlate(const InputParameters & params);

  virtual void addMooseObjects() override;
  virtual Real getUnitPerimeter(const MooseEnum & side) const override;

protected:
  /// plate fuel depth
  const Real & _depth;
};
