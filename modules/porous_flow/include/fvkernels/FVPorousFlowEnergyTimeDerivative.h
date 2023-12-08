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
class FVPorousFlowEnergyTimeDerivative : public FVTimeKernel
{
public:
  static InputParameters validParams();
  FVPorousFlowEnergyTimeDerivative(const InputParameters & parameters);

protected:
  ADReal computeQpResidual() override;

  const PorousFlowDictator & _dictator;
  const unsigned int _num_phases;
  const bool _fluid_present;

  const ADMaterialProperty<Real> & _porosity;
  const ADMaterialProperty<std::vector<Real>> * const _density;
  const ADMaterialProperty<Real> & _rock_energy;
  const ADMaterialProperty<std::vector<Real>> * const _energy;
  const ADMaterialProperty<std::vector<Real>> * const _saturation;
  const MaterialProperty<Real> & _porosity_old;
  const MaterialProperty<std::vector<Real>> * const _density_old;
  const MaterialProperty<Real> & _rock_energy_old;
  const MaterialProperty<std::vector<Real>> * const _energy_old;
  const MaterialProperty<std::vector<Real>> * const _saturation_old;
};
