//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef POROUSFLOWPOLYLINESINK
#define POROUSFLOWPOLYLINESINK

#include "PorousFlowLineSink.h"
#include "LinearInterpolation.h"

class PorousFlowPolyLineSink;

template <>
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
  void computeQpBaseOutflowJacobian(unsigned jvar,
                                    unsigned current_dirac_ptid,
                                    Real & outflow,
                                    Real & outflowp) const override;
};

#endif // POROUSFLOWPOLYLINESINK
