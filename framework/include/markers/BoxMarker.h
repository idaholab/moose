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

#include "libmesh/bounding_box.h"

class BoxMarker : public Marker
{
public:
  static InputParameters validParams();

  BoxMarker(const InputParameters & parameters);

protected:
  virtual MarkerValue computeElementMarker() override;

  MarkerValue _inside;
  MarkerValue _outside;

  BoundingBox _bounding_box;
};
