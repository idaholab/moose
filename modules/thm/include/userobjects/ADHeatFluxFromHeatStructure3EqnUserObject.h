#pragma once

#include "ADHeatFluxFromHeatStructureBaseUserObject.h"

/**
 * Cache the heat flux between a single phase flow channel and a heat structure
 */
class ADHeatFluxFromHeatStructure3EqnUserObject : public ADHeatFluxFromHeatStructureBaseUserObject
{
public:
  ADHeatFluxFromHeatStructure3EqnUserObject(const InputParameters & parameters);

protected:
  virtual ADReal computeQpHeatFlux() override;

  const ADMaterialProperty<Real> & _T_wall;
  const ADMaterialProperty<Real> & _Hw;
  const ADMaterialProperty<Real> & _T;

public:
  static InputParameters validParams();
};
