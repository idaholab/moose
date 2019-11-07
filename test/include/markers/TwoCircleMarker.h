//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Marker.h"

#include "libmesh/mesh_tools.h"

class TwoCircleMarker : public Marker
{
public:
  static InputParameters validParams();

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
