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
 * Postprocessor produces the sum of heat energy of the porous skeleton and/or fluid components in a
 * region
 */
class PorousFlowHeatEnergy : public ElementIntegralPostprocessor
{
public:
  static InputParameters validParams();

  PorousFlowHeatEnergy(const InputParameters & parameters);

protected:
  virtual Real computeIntegral() override;
  virtual Real computeQpIntegral() override;

  /// PorousFlowDictator UserObject
  const PorousFlowDictator & _dictator;

  /// Number of fluid phases
  const unsigned int _num_phases;

  /// base name used in the Tensor Mechanics strain calculator
  const std::string _base_name;
  /// Whether there is a Material called _base_name_total_strain
  const bool _has_total_strain;
  /// Value of total strain calculated by a Tensor Mechanics strain calculator, if it exists, otherwise nullptr
  const MaterialProperty<RankTwoTensor> * const _total_strain;

  /// Whether fluid is present
  const bool _fluid_present;

  /// Whether to include the heat energy of the porous skeleton in the calculations
  const bool _include_porous_skeleton;

  /// The phase indices that this Postprocessor is restricted to
  std::vector<unsigned int> _phase_index;

  /// Porosity
  const MaterialProperty<Real> & _porosity;

  /// Nodal rock energy density
  const MaterialProperty<Real> & _rock_energy_nodal;

  /// Nodal fluid density
  const MaterialProperty<std::vector<Real>> * const _fluid_density;

  /// Nodal fluid saturation
  const MaterialProperty<std::vector<Real>> * const _fluid_saturation_nodal;

  /// Internal energy of the phases, evaluated at the nodes
  const MaterialProperty<std::vector<Real>> * const _energy_nodal;

  /// The variable for the corresponding PorousFlowEnergyTimeDerivative Kernel: this provides test functions
  MooseVariable * const _var;
};
