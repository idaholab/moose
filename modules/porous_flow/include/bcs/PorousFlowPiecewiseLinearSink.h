/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#ifndef POROUSFLOWPIECEWISELINEARSINK_H
#define POROUSFLOWPIECEWISELINEARSINK_H

#include "PorousFlowSink.h"
#include "LinearInterpolation.h"

// Forward Declarations
class PorousFlowPiecewiseLinearSink;

template<>
InputParameters validParams<PorousFlowPiecewiseLinearSink>();

/**
 * Applies a flux sink to a boundary.  The base flux
 * defined by PorousFlowSink is multiplied by a
 * piecewise linear function of porepressure evaluated
 * at the quad points.
 */
class PorousFlowPiecewiseLinearSink : public PorousFlowSink
{
public:

  PorousFlowPiecewiseLinearSink(const InputParameters & parameters);

protected:
  /// piecewise-linear function of porepressure that multiplies the sink flux
  const LinearInterpolation _sink_func;

  /// Nodal pore pressure in each phase
  const MaterialProperty<std::vector<Real> > & _pp;

  /// d(Nodal pore pressure in each phase)/d(PorousFlow variable)
  const MaterialProperty<std::vector<std::vector<Real> > > & _dpp_dvar;

  virtual Real multiplier();

  virtual Real dmultiplier_dvar(unsigned int pvar);
};

#endif //POROUSFLOWPIECEWISELINEARSINK_H
