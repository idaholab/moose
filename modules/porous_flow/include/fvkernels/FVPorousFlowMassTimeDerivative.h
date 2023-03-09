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
class FVPorousFlowMassTimeDerivative : public FVTimeKernel
{
public:
  static InputParameters validParams();
  FVPorousFlowMassTimeDerivative(const InputParameters & parameters);

protected:
  ADReal computeQpResidual() override;

  const PorousFlowDictator & _dictator;
  const unsigned int _num_phases;
  const unsigned int _fluid_component;

  const ADMaterialProperty<Real> & _porosity;
  const ADMaterialProperty<std::vector<Real>> & _density;
  const ADMaterialProperty<std::vector<Real>> & _saturation;
  const ADMaterialProperty<std::vector<std::vector<Real>>> & _mass_fractions;
  const MaterialProperty<Real> & _porosity_old;
  const MaterialProperty<std::vector<Real>> & _density_old;
  const MaterialProperty<std::vector<Real>> & _saturation_old;
  const MaterialProperty<std::vector<std::vector<Real>>> & _mass_fractions_old;
};
