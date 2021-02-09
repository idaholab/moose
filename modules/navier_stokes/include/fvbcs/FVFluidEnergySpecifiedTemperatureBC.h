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

class SinglePhaseFluidProperties;

/**
 *
 */
class CNSFVFluidEnergySpecifiedTemperatureBC : public FVFluxBC
{
public:
  CNSFVFluidEnergySpecifiedTemperatureBC(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  virtual ADReal computeQpResidual() override;

  // porosity
  // const VariableValue & _eps;
  const Real _eps;

  /// specified temperature
  const PostprocessorValue & _temperature;

  /// velocity
  const ADMaterialProperty<RealVectorValue> & _velocity;

  /// pressure
  const ADMaterialProperty<Real> & _pressure;

  /// fluid properties
  const SinglePhaseFluidProperties & _fluid;
};
