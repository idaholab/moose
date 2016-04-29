/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWSQUAREPULSEPOINTSOURCE_H
#define POROUSFLOWSQUAREPULSEPOINTSOURCE_H

#include "DiracKernel.h"

class PorousFlowSquarePulsePointSource;

template<>
InputParameters validParams<PorousFlowSquarePulsePointSource>();

/**
 * Point source (or sink) that adds (removes) fluid at a constant rate for times
 * less than the specified end time. If no end time is specified, the source (sink)
 * continues to act indefinately.
 */
class PorousFlowSquarePulsePointSource : public DiracKernel
{
public:
  PorousFlowSquarePulsePointSource(const InputParameters & parameters);

  virtual void addPoints();

  virtual Real computeQpResidual();

protected:
  /// The constant mass flux (kg/s)
  Real _mass_flux;

  /// The location of the point source (sink)
  Point _p;

  /// The time at which the point source (sink) starts operating
  Real _start_time;

  /// The time at which the point source (sink) stops operating
  Real _end_time;
};

#endif //POROUSFLOWSQUAREPULSEPOINTSOURCE_H
