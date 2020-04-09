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

#include "RichardsDensity.h"

// Forward Declarations

/**
 * Fluid density as a function of porepressure
 */
class RichardsDensityAux : public AuxKernel
{
public:
  static InputParameters validParams();

  RichardsDensityAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  /// porepressure
  const VariableValue & _pressure_var;

  /// userobject that defines density as a fcn of porepressure
  const RichardsDensity & _density_UO;
};
