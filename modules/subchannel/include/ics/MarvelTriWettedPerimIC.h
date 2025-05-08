//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TriSubChannelBaseIC.h"

/**
 * Sets the wetted perimeter of the triangular, edge, and corner subchannels for a MARVEL-type
 * microreactor design
 */
class MarvelTriWettedPerimIC : public TriSubChannelBaseIC
{
public:
  MarvelTriWettedPerimIC(const InputParameters & params);
  Real value(const Point & p) override;

public:
  static InputParameters validParams();
};
