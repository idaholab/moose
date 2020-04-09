//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PorousFlowSinkPTDefiner.h"
#include "LinearInterpolation.h"

/**
 * Applies a flux sink to a boundary.  The base flux
 * defined by PorousFlowSink is multiplied by a
 * piecewise linear function of porepressure (or temperature for
 * the case of a BC with heat and no fluid)
 * evaluated at the quad points.
 */
class PorousFlowPiecewiseLinearSink : public PorousFlowSinkPTDefiner
{
public:
  static InputParameters validParams();

  PorousFlowPiecewiseLinearSink(const InputParameters & parameters);

protected:
  /// Piecewise-linear function of porepressure that multiplies the sink flux
  const LinearInterpolation _sink_func;

  virtual Real multiplier() const override;

  virtual Real dmultiplier_dvar(unsigned int pvar) const override;
};
