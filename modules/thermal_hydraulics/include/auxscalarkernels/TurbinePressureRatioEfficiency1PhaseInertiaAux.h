//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxScalarKernel.h"

class ADTurbinePressureRatioEfficiency1PhaseUserObject;

/**
 * Moment of inertia computed in the 1-phase pressure ratio and efficiency turbine
 */
class TurbinePressureRatioEfficiency1PhaseInertiaAux : public AuxScalarKernel
{
public:
  TurbinePressureRatioEfficiency1PhaseInertiaAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();
  /// 1-phase pressure ratio and efficiency turbine
  const ADTurbinePressureRatioEfficiency1PhaseUserObject & _turbine_uo;

public:
  static InputParameters validParams();
};
