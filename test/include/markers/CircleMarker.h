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

#ifndef CIRCLEMARKER_H
#define CIRCLEMARKER_H

#include "Marker.h"

// libmesh includes
#include "libmesh/mesh_tools.h"

class CircleMarker;

template <>
InputParameters validParams<CircleMarker>();

class CircleMarker : public Marker
{
public:
  CircleMarker(const InputParameters & parameters);
  virtual ~CircleMarker(){};

protected:
  virtual MarkerValue computeElementMarker();

  MarkerValue _inside;
  MarkerValue _outside;

  Point _p;
  Real _r;
};

#endif /* CIRCLEMARKER_H */
