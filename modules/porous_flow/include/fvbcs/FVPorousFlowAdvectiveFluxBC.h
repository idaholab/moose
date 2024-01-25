//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVFluxBC.h"

class PorousFlowDictator;

/**
 * Flux boundary condition where an advective flux is applied
 */
class FVPorousFlowAdvectiveFluxBC : public FVFluxBC
{
public:
  static InputParameters validParams();
  FVPorousFlowAdvectiveFluxBC(const InputParameters & params);

protected:
  virtual ADReal computeQpResidual() override;

  /// UserObject that holds information (number of phases, components, etc)
  const PorousFlowDictator & _dictator;
  /// Number of fluid phases present
  const unsigned int _num_phases;
  /// Index of the fluid phase this BC applies to
  const unsigned int _phase;
  /// Index of the fluid component this BC applies to
  const unsigned int _fluid_component;

  /// Fluid density
  const ADMaterialProperty<std::vector<Real>> & _density;
  const ADMaterialProperty<std::vector<Real>> & _density_neighbor;

  /// Fluid viscosity
  const ADMaterialProperty<std::vector<Real>> & _viscosity;
  const ADMaterialProperty<std::vector<Real>> & _viscosity_neighbor;

  /// Relative permeability
  const ADMaterialProperty<std::vector<Real>> & _relperm;
  const ADMaterialProperty<std::vector<Real>> & _relperm_neighbor;

  /// Mass fraction of fluid components in fluid phases
  const ADMaterialProperty<std::vector<std::vector<Real>>> & _mass_fractions;
  const ADMaterialProperty<std::vector<std::vector<Real>>> & _mass_fractions_neighbor;

  /// Permeability
  const ADMaterialProperty<RealTensorValue> & _permeability;
  const ADMaterialProperty<RealTensorValue> & _permeability_neighbor;

  /// Fluid pressure
  const ADMaterialProperty<std::vector<Real>> & _pressure;
  const ADMaterialProperty<std::vector<Real>> & _pressure_neighbor;

  /// Gravity vector
  const RealVectorValue & _gravity;

  /// The porepressure value at the boundary
  const Real _pp_value;
};
