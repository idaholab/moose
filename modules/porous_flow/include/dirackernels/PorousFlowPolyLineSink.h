/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWPOLYLINESINK
#define POROUSFLOWPOLYLINESINK

#include "PorousFlowLineSink.h"
#include "LinearInterpolation.h"

class PorousFlowPolyLineSink;

template<>
InputParameters validParams<PorousFlowPolyLineSink>();

/**
 * Approximates a line sink by a sequence of Dirac Points
 */
class PorousFlowPolyLineSink : public PorousFlowLineSink
{
public:
  PorousFlowPolyLineSink(const InputParameters & parameters);

protected:
  /// mass flux = _sink_func as a function of porepressure or temperature
  LinearInterpolation _sink_func;

  Real computeQpBaseOutflow(unsigned current_dirac_ptid) const override;
  void computeQpBaseOutflowJacobian(unsigned jvar, unsigned current_dirac_ptid, Real & outflow, Real & outflowp) const override;

};

#endif //POROUSFLOWPOLYLINESINK
