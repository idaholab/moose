/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWPIECEWISELINEARSINK_H
#define POROUSFLOWPIECEWISELINEARSINK_H

#include "PorousFlowSinkPTDefiner.h"
#include "LinearInterpolation.h"

// Forward Declarations
class PorousFlowPiecewiseLinearSink;

template <>
InputParameters validParams<PorousFlowPiecewiseLinearSink>();

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
  PorousFlowPiecewiseLinearSink(const InputParameters & parameters);

protected:
  /// piecewise-linear function of porepressure that multiplies the sink flux
  const LinearInterpolation _sink_func;

  virtual Real multiplier() const override;

  virtual Real dmultiplier_dvar(unsigned int pvar) const override;
};

#endif // POROUSFLOWPIECEWISELINEARSINK_H
