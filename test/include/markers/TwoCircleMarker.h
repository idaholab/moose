/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef TWOCIRCLEMARKER_H
#define TWOCIRCLEMARKER_H

#include "Marker.h"

#include "libmesh/mesh_tools.h"

class TwoCircleMarker;

template <>
InputParameters validParams<TwoCircleMarker>();

class TwoCircleMarker : public Marker
{
public:
  TwoCircleMarker(const InputParameters & parameters);
  virtual ~TwoCircleMarker(){};

protected:
  virtual MarkerValue computeElementMarker();

  const MarkerValue _inside;
  const MarkerValue _outside;

  const Point _p1;
  const Real _r1;
  const Point _p2;
  const Real _r2;
  const Real _shut_off_time;
};

#endif /* TWOCIRCLEMARKER_H */
