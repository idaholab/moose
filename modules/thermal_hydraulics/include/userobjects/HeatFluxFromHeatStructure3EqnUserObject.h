//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "HeatFluxFromHeatStructureBaseUserObject.h"
#include "DerivativeMaterialInterfaceTHM.h"

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

public:
  static InputParameters validParams();
};
