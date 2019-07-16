#pragma once

#include "HeatFluxFromHeatStructureBaseUserObject.h"
#include "DerivativeMaterialInterfaceTHM.h"

class HeatFluxFromHeatStructure3EqnUserObject;

template <>
InputParameters validParams<HeatFluxFromHeatStructure3EqnUserObject>();

/**
 * Cache the heat flux between a single phase flow channel and a heat structure
 */
class HeatFluxFromHeatStructure3EqnUserObject
  : public DerivativeMaterialInterfaceTHM<HeatFluxFromHeatStructureBaseUserObject>
{
public:
  HeatFluxFromHeatStructure3EqnUserObject(const InputParameters & parameters);

protected:
  virtual Real computeQpHeatFlux() override;
  virtual DenseVector<Real> computeQpHeatFluxJacobian() override;

  const VariableValue & _T_wall;

  const MaterialProperty<Real> & _Hw;
  const MaterialProperty<Real> & _T;
  const MaterialProperty<Real> & _dT_drhoA;
  const MaterialProperty<Real> & _dT_drhouA;
  const MaterialProperty<Real> & _dT_drhoEA;
};
