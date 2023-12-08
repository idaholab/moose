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

class PorousFlowDictator;

class FVPorousFlowHeatAdvection : public FVFluxKernel
{
public:
  static InputParameters validParams();
  FVPorousFlowHeatAdvection(const InputParameters & params);

protected:
  virtual ADReal computeQpResidual() override;

  const PorousFlowDictator & _dictator;
  const unsigned int _num_phases;

  const ADMaterialProperty<std::vector<Real>> & _density;
  const ADMaterialProperty<std::vector<Real>> & _density_neighbor;

  const ADMaterialProperty<std::vector<Real>> & _viscosity;
  const ADMaterialProperty<std::vector<Real>> & _viscosity_neighbor;

  const ADMaterialProperty<std::vector<Real>> & _enthalpy;
  const ADMaterialProperty<std::vector<Real>> & _enthalpy_neighbor;

  const ADMaterialProperty<std::vector<Real>> & _relperm;
  const ADMaterialProperty<std::vector<Real>> & _relperm_neighbor;

  const ADMaterialProperty<RealTensorValue> & _permeability;
  const ADMaterialProperty<RealTensorValue> & _permeability_neighbor;

  const ADMaterialProperty<std::vector<Real>> & _pressure;
  const ADMaterialProperty<std::vector<Real>> & _pressure_neighbor;
  const ADMaterialProperty<std::vector<RealGradient>> & _grad_p;

  const RealVectorValue & _gravity;
};
