//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVTimeKernel.h"

class PorousFlowDictator;

/**
 * Time derivative of energy
 */
class FVPorousFlowEnergyTimeDerivative : public FVTimeKernel
{
public:
  static InputParameters validParams();
  FVPorousFlowEnergyTimeDerivative(const InputParameters & parameters);

protected:
  ADReal computeQpResidual() override;

  /// UserObject that holds information (number of phases, components, etc)
  const PorousFlowDictator & _dictator;
  /// Number of fluid phases
  const unsigned int _num_phases;
  /// Whether fluid is present
  const bool _fluid_present;

  /// Porosity
  const ADMaterialProperty<Real> & _porosity;
  const MaterialProperty<Real> & _porosity_old;

  /// Fluid density
  const ADMaterialProperty<std::vector<Real>> * const _density;
  const MaterialProperty<std::vector<Real>> * const _density_old;

  /// Internal energy of porous matrix
  const ADMaterialProperty<Real> & _rock_energy;
  const MaterialProperty<Real> & _rock_energy_old;

  /// Internal energy of fluid
  const ADMaterialProperty<std::vector<Real>> * const _energy;
  const MaterialProperty<std::vector<Real>> * const _energy_old;

  /// Fluid phase saturation
  const ADMaterialProperty<std::vector<Real>> * const _saturation;
  const MaterialProperty<std::vector<Real>> * const _saturation_old;
};
