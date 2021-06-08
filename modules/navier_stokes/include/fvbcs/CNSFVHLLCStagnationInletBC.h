//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CNSFVHLLCBC.h"

/**
 * Base class for the HLLC stagnation inlet boundary conditions
 */
class CNSFVHLLCStagnationInletBC : public CNSFVHLLCBC
{
public:
  CNSFVHLLCStagnationInletBC(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  virtual void preComputeWaveSpeed() override;

  /// stagnation temperature
  const PostprocessorValue & _stagnation_temperature;

  /// stagnation pressure
  const PostprocessorValue & _stagnation_pressure;

  /// isobaric specific heat
  const ADMaterialProperty<Real> & _cp;

  /// isochoric specific heat
  const ADMaterialProperty<Real> & _cv;

  /// pressure on the boundary side
  ADReal _p_boundary;

  /// enthalpy on the boundary side
  ADReal _ht_boundary;
};
