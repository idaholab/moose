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
 * Advective flux of heat energy
 */
class FVPorousFlowHeatAdvection : public FVFluxKernel
{
public:
  static InputParameters validParams();
  FVPorousFlowHeatAdvection(const InputParameters & params);

protected:
  virtual ADReal computeQpResidual() override;

  /// UserObject that holds information (number of phases, components, etc)
  const PorousFlowDictator & _dictator;
  /// Number of fluid phases present
  const unsigned int _num_phases;

  /// Fluid density
  const ADMaterialProperty<std::vector<Real>> & _density;
  const ADMaterialProperty<std::vector<Real>> & _density_neighbor;

  /// Fluid viscosity
  const ADMaterialProperty<std::vector<Real>> & _viscosity;
  const ADMaterialProperty<std::vector<Real>> & _viscosity_neighbor;

  /// Fluid enthalpy
  const ADMaterialProperty<std::vector<Real>> & _enthalpy;
  const ADMaterialProperty<std::vector<Real>> & _enthalpy_neighbor;

  /// Relative permeability
  const ADMaterialProperty<std::vector<Real>> & _relperm;
  const ADMaterialProperty<std::vector<Real>> & _relperm_neighbor;

  /// Permeability
  const ADMaterialProperty<RealTensorValue> & _permeability;
  const ADMaterialProperty<RealTensorValue> & _permeability_neighbor;

  /// Fluid pressure
  const ADMaterialProperty<std::vector<Real>> & _pressure;
  const ADMaterialProperty<std::vector<Real>> & _pressure_neighbor;
  const ADMaterialProperty<std::vector<RealGradient>> & _grad_p;

  /// Gravity vector
  const RealVectorValue & _gravity;
};
