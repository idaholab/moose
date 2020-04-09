//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SideAverageValue.h"

// Forward Declarations

/**
 * This postprocessor computes the thermal conductivity of the bulk.
 */
class ThermalConductivity : public SideAverageValue
{
public:
  static InputParameters validParams();

  ThermalConductivity(const InputParameters & parameters);

  virtual Real getValue();

protected:
  const Real _dx;
  const PostprocessorValue & _flux;
  const PostprocessorValue & _T_hot;
  const Real _length_scale;
  const Real _k0;

private:
  /// True if this is the zeroth timestep (timestep < 1). At the zero
  /// timestep, the initial value of thermal conductivity should be returned.
  /// This boolean is delcared as a reference so that the variable is restartable
  /// data:  if we restart, the code will not think it is the zero timestep again.
  bool & _step_zero;
};
