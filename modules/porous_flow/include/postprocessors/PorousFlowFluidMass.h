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
 * Postprocessor produces the mass of a given fluid component in a region.
 *
 * This Postprocessor is templated to work with both AD and non-AD FE
 * variables, as well as FV variables. For the FE case, the Postprocessor
 * is lumped to the nodes (like the time derivative kernel). For this reason,
 * the fluid mass for the FE case is computed in computeIntegral() using a
 * nodal sum. The FV case is much simpler, so the fluid mass is computed in
 * computeQpIntegral(), which is called through the base
 * ElementIntegralPostprocessor::computeIntegral().
 */
template <bool is_ad>
class PorousFlowFluidMassTempl : public ElementIntegralPostprocessor
{
public:
  static InputParameters validParams();

  PorousFlowFluidMassTempl(const InputParameters & parameters);

protected:
  virtual Real computeIntegral() override;
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
  const GenericMaterialProperty<Real, is_ad> & _porosity;
  /// Phase density (kg/m^3)
  const GenericMaterialProperty<std::vector<Real>, is_ad> & _fluid_density;
  /// Phase saturation (-)
  const GenericMaterialProperty<std::vector<Real>, is_ad> & _fluid_saturation;
  /// Mass fraction of each fluid component in each phase
  const GenericMaterialProperty<std::vector<std::vector<Real>>, is_ad> & _mass_fraction;
  /// Saturation threshold - only fluid mass at saturations below this are calculated
  const Real _saturation_threshold;
  /// The variable for the corresponding PorousFlowMassTimeDerivative Kernel: this provides test functions
  MooseVariable * const _var;
};

typedef PorousFlowFluidMassTempl<false> PorousFlowFluidMass;
typedef PorousFlowFluidMassTempl<true> ADPorousFlowFluidMass;
