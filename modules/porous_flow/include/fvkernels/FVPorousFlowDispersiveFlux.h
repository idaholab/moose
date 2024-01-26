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
#include "RankTwoTensor.h"

class PorousFlowDictator;

/**
 * Dispersive flux of component k in fluid phase alpha. Includes the effects
 * of both molecular diffusion and hydrodynamic dispersion
 */
class FVPorousFlowDispersiveFlux : public FVFluxKernel
{
public:
  static InputParameters validParams();
  FVPorousFlowDispersiveFlux(const InputParameters & params);

protected:
  virtual ADReal computeQpResidual() override;

  /// UserObject that holds information (number of phases, components, etc)
  const PorousFlowDictator & _dictator;
  /// Number of fluid phases present
  const unsigned int _num_phases;
  /// Index of the fluid component this kernel applies to
  const unsigned int _fluid_component;

  /// Fluid density
  const ADMaterialProperty<std::vector<Real>> & _density;

  /// Fluid viscosity
  const ADMaterialProperty<std::vector<Real>> & _viscosity;
  const ADMaterialProperty<std::vector<Real>> & _viscosity_neighbor;

  /// Relative permeability
  const ADMaterialProperty<std::vector<Real>> & _relperm;
  const ADMaterialProperty<std::vector<Real>> & _relperm_neighbor;

  /// Mass fraction of fluid components in fluid phases
  const ADMaterialProperty<std::vector<std::vector<Real>>> & _mass_fractions;
  const ADMaterialProperty<std::vector<std::vector<Real>>> & _mass_fractions_neighbor;
  const ADMaterialProperty<std::vector<std::vector<RealGradient>>> & _grad_mass_frac;

  /// Permeability
  const ADMaterialProperty<RealTensorValue> & _permeability;
  const ADMaterialProperty<RealTensorValue> & _permeability_neighbor;

  /// Fluid pressure
  const ADMaterialProperty<std::vector<Real>> & _pressure;
  const ADMaterialProperty<std::vector<Real>> & _pressure_neighbor;
  const ADMaterialProperty<std::vector<RealGradient>> & _grad_p;

  /// Porosity
  const ADMaterialProperty<Real> & _porosity;
  const ADMaterialProperty<Real> & _porosity_neighbor;

  /// Tortuosity tau_0 * tau_{alpha} for fluid phase alpha
  const ADMaterialProperty<std::vector<Real>> & _tortuosity;
  const ADMaterialProperty<std::vector<Real>> & _tortuosity_neighbor;

  /// Diffusion coefficients of component k in fluid phase alpha
  const MaterialProperty<std::vector<std::vector<Real>>> & _diffusion_coeff;
  const MaterialProperty<std::vector<std::vector<Real>>> & _diffusion_coeff_neighbor;

  /// Gravity vector
  const RealVectorValue & _gravity;
  const ADRankTwoTensor _identity_tensor;

  /// Longitudinal dispersivity for each phase
  const std::vector<Real> _disp_long;

  /// Transverse dispersivity for each phase
  const std::vector<Real> _disp_trans;
};
