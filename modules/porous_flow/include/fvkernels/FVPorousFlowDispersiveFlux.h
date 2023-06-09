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

class FVPorousFlowDispersiveFlux : public FVFluxKernel
{
public:
  static InputParameters validParams();
  FVPorousFlowDispersiveFlux(const InputParameters & params);

protected:
  virtual ADReal computeQpResidual() override;

  const PorousFlowDictator & _dictator;
  const unsigned int _num_phases;
  const unsigned int _fluid_component;

  const ADMaterialProperty<std::vector<Real>> & _density;

  const ADMaterialProperty<std::vector<Real>> & _viscosity;
  const ADMaterialProperty<std::vector<Real>> & _viscosity_neighbor;

  const ADMaterialProperty<std::vector<Real>> & _relperm;
  const ADMaterialProperty<std::vector<Real>> & _relperm_neighbor;

  const ADMaterialProperty<std::vector<std::vector<Real>>> & _mass_fractions;
  const ADMaterialProperty<std::vector<std::vector<Real>>> & _mass_fractions_neighbor;
  const ADMaterialProperty<std::vector<std::vector<RealGradient>>> & _grad_mass_frac;

  const ADMaterialProperty<RealTensorValue> & _permeability;
  const ADMaterialProperty<RealTensorValue> & _permeability_neighbor;

  const ADMaterialProperty<std::vector<Real>> & _pressure;
  const ADMaterialProperty<std::vector<Real>> & _pressure_neighbor;
  const ADMaterialProperty<std::vector<RealGradient>> & _grad_p;

  /// Porosity at the qps
  const ADMaterialProperty<Real> & _porosity;
  const ADMaterialProperty<Real> & _porosity_neighbor;

  /// Tortuosity tau_0 * tau_{alpha} for fluid phase alpha
  const ADMaterialProperty<std::vector<Real>> & _tortuosity;
  const ADMaterialProperty<std::vector<Real>> & _tortuosity_neighbor;

  /// Diffusion coefficients of component k in fluid phase alpha
  const MaterialProperty<std::vector<std::vector<Real>>> & _diffusion_coeff;
  const MaterialProperty<std::vector<std::vector<Real>>> & _diffusion_coeff_neighbor;

  const RealVectorValue & _gravity;
  const ADRankTwoTensor _identity_tensor;

  /// Longitudinal dispersivity for each phase
  const std::vector<Real> _disp_long;

  /// Transverse dispersivity for each phase
  const std::vector<Real> _disp_trans;
};
