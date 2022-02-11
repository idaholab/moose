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
 * Computes convective heat flux for 1-phase flow
 */
class ConvectiveHeatFlux1PhaseAux : public AuxKernel
{
public:
  ConvectiveHeatFlux1PhaseAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  /// Wall temperature
  const VariableValue & _T_wall;
  /// Fluid temperature
  const MaterialProperty<Real> & _T;
  /// Wall heat transfer coefficient
  const MaterialProperty<Real> & _Hw;
  /// Scaling factor
  const Real & _scaling_factor;

public:
  static InputParameters validParams();
};
