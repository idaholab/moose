//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementIntegralVariablePostprocessor.h"
#include "PorousFlowDictator.h"

/**
 * Postprocessor that calculates the mass of a given fluid component in the given phase(s)
 * in a given block
 */
class FVPorousFlowFluidMass : public ElementIntegralPostprocessor
{
public:
  static InputParameters validParams();

  FVPorousFlowFluidMass(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral() override;

  /// PorousFlowDictator UserObject
  const PorousFlowDictator & _dictator;
  /// The fluid component index that this Postprocessor applies to
  const unsigned int _fluid_component;
  /// The phase indices that this Postprocessor is restricted to
  std::vector<unsigned int> _phase_index;
  /// base name used in the Tensor Mechanics strain calculator
  const std::string _base_name;
  /// Whether there is a Material called _base_name_total_strain
  const bool _has_total_strain;
  /// Value of total strain calculated by a Tensor Mechanics strain calculator, if it exists, otherwise nullptr
  const MaterialProperty<RankTwoTensor> * const _total_strain;
  /// Porosity
  const ADMaterialProperty<Real> & _porosity;
  /// Phase density (kg/m^3)
  const ADMaterialProperty<std::vector<Real>> & _fluid_density;
  /// Phase saturation (-)
  const ADMaterialProperty<std::vector<Real>> & _fluid_saturation;
  /// Mass fraction of each fluid component in each phase
  const ADMaterialProperty<std::vector<std::vector<Real>>> & _mass_fraction;
  /// Saturation threshold - only fluid mass at saturations below this are calculated
  const Real _saturation_threshold;
};
