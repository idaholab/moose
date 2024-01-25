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
 * Time derivative of fluid mass
 */
class FVPorousFlowMassTimeDerivative : public FVTimeKernel
{
public:
  static InputParameters validParams();
  FVPorousFlowMassTimeDerivative(const InputParameters & parameters);

protected:
  ADReal computeQpResidual() override;

  /// UserObject that holds information (number of phases, components, etc)
  const PorousFlowDictator & _dictator;
  /// Number of fluid phases present
  const unsigned int _num_phases;
  /// Index of the fluid component this kernel applies to
  const unsigned int _fluid_component;

  /// Porosity
  const ADMaterialProperty<Real> & _porosity;
  const MaterialProperty<Real> & _porosity_old;

  /// Fluid density
  const ADMaterialProperty<std::vector<Real>> & _density;
  const MaterialProperty<std::vector<Real>> & _density_old;

  /// Fluid phase saturation
  const ADMaterialProperty<std::vector<Real>> & _saturation;
  const MaterialProperty<std::vector<Real>> & _saturation_old;

  /// Mass fraction of fluid components in fluid phases
  const ADMaterialProperty<std::vector<std::vector<Real>>> & _mass_fractions;
  const MaterialProperty<std::vector<std::vector<Real>>> & _mass_fractions_old;
};
