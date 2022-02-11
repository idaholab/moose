//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

class SinglePhaseFluidProperties;

/**
 * Computes Reynolds number
 */
class ReynoldsNumberAux : public AuxKernel
{
public:
  ReynoldsNumberAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  /// Volume fraction
  const VariableValue & _alpha;
  /// Density of the phase
  const VariableValue & _rho;
  /// Velocity of the phase
  const VariableValue & _vel;
  /// Hydraulic diameter
  const VariableValue & _D_h;
  /// Specific volume
  const VariableValue & _v;
  /// Specific internal energy
  const VariableValue & _e;

  const SinglePhaseFluidProperties & _fp;

public:
  static InputParameters validParams();
};
