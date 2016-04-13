/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWTIMELIMITEDCONSTANTPOINTSOURCE_H
#define POROUSFLOWTIMELIMITEDCONSTANTPOINTSOURCE_H

#include "DiracKernel.h"

class PorousFlowTimeLimitedConstantPointSource;

template<>
InputParameters validParams<PorousFlowTimeLimitedConstantPointSource>();

/**
 * Point source (or sink) that adds (removes) fluid at a constant rate for times
 * less than the specified end time. If no end time is specified, the source (sink)
 * continues to act indefinately.
 */
class PorousFlowTimeLimitedConstantPointSource : public DiracKernel
{
public:
  PorousFlowTimeLimitedConstantPointSource(const InputParameters & parameters);

  virtual void addPoints();
  virtual Real computeQpResidual();

protected:
  /// The constant mass flux (kg/s)
  Real _mass_flux;
  /// Vector of points that specify the location of the point source (sink)
  std::vector<Real> _point_param;
  /// The location of the point source (sink)
  Point _p;
  /// The time at which the point source (sink) stops operating
  Real _end_time;
};

#endif //POROUSFLOWTIMELIMITEDCONSTANTPOINTSOURCE_H
