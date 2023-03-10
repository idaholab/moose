//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeExtraStressBase.h"

/**
 * Sets up an isotropic extra Stress with a trace that equals the pressure of
 * an ideal gas with the given density.
 * \f$ PV=Nk_BT \f$
 */
class ComputeGasPressureBase : public ComputeExtraStressBase
{
public:
  static InputParameters validParams();

  ComputeGasPressureBase(const InputParameters & parameters);

protected:
  /// concentration
  const VariableValue & _c;

  /// temperature
  const VariableValue & _T;

  /// lattice site volume
  const Real _omega;

  // Pressure unit conversion factor
  const Real _pressure_unit_conversion;

  // Boltzmann constant
  const Real _kB;
};
