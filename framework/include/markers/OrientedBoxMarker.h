//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "Marker.h"
#include "OrientedBoxInterface.h"

/**
 * Creates a box of specified width, length and height,
 * with its center at specified position,
 * and with the direction along the width direction specified,
 * and with the direction along the length direction specified.
 * Then elements are marked as inside or outside this box
 */
class OrientedBoxMarker : public Marker, public OrientedBoxInterface
{
public:
  static InputParameters validParams();

  OrientedBoxMarker(const InputParameters & parameters);

protected:
  virtual MarkerValue computeElementMarker() override;

  MarkerValue _inside;
  MarkerValue _outside;
};
