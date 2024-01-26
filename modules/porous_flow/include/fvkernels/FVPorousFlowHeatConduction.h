//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVFluxKernel.h"

class PorousFlowDictator;

/**
 * Heat conduction kernel
 */
class FVPorousFlowHeatConduction : public FVFluxKernel
{
public:
  static InputParameters validParams();
  FVPorousFlowHeatConduction(const InputParameters & params);

protected:
  virtual ADReal computeQpResidual() override;

  /// UserObject that holds information (number of phases, components, etc)
  const PorousFlowDictator & _dictator;

  /// Thermal conductivity
  const ADMaterialProperty<RealTensorValue> & _lambda_element;
  const ADMaterialProperty<RealTensorValue> & _lambda_neighbor;

  /// Temperature
  const ADMaterialProperty<Real> & _temperature_element;
  const ADMaterialProperty<Real> & _temperature_neighbor;

  /// Temperature gradient
  const ADMaterialProperty<RealGradient> & _grad_T;
};
