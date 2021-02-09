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
 * Stagnation inlet BC for the the mass equation
 */
class CNSFVStagnationInletBC : public FVFluxBC
{
public:
  CNSFVStagnationInletBC(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  /// helper function that computes inlet conditions from stagnation p, T
  void inletConditionHelper(ADReal & p_inlet,
                            ADReal & T_inlet,
                            ADReal & rho_inlet,
                            ADReal & H_inlet) const;

  /// porosity
  // const VariableValue & _eps;
  const Real _eps;

  /// stagnation temperature
  const PostprocessorValue & _stagnation_temperature;

  /// stagnation pressure
  const PostprocessorValue & _stagnation_pressure;

  /// velocity
  const ADMaterialProperty<RealVectorValue> & _velocity;

  /// speed
  const ADMaterialProperty<Real> & _speed;

  /// isobaric specific heat
  const ADMaterialProperty<Real> & _cp;

  /// isochoric specific heat
  const ADMaterialProperty<Real> & _cv;

  /// fluid properties
  const SinglePhaseFluidProperties & _fluid;

  /// is the fluid property object for ideal gases
  bool _fluid_ideal_gas;
};
