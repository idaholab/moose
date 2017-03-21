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

template <>
InputParameters validParams<PorousFlowSquarePulsePointSource>();

/**
 * Point source (or sink) that adds (removes) fluid at a constant mass flux rate for times
 * between the specified start and end times. If no start and end times are specified,
 * the source (sink) starts at the start of the simulation and continues to act indefinitely
 */
class PorousFlowSquarePulsePointSource : public DiracKernel
{
public:
  PorousFlowSquarePulsePointSource(const InputParameters & parameters);

  virtual void addPoints() override;
  virtual Real computeQpResidual() override;

protected:
  /// The constant mass flux (kg/s)
  const Real _mass_flux;

  /// The location of the point source (sink)
  const Point _p;

  /// The time at which the point source (sink) starts operating
  const Real _start_time;

  /// The time at which the point source (sink) stops operating
  const Real _end_time;
};

#endif // POROUSFLOWSQUAREPULSEPOINTSOURCE_H
