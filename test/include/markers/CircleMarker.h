//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef CIRCLEMARKER_H
#define CIRCLEMARKER_H

#include "Marker.h"

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
