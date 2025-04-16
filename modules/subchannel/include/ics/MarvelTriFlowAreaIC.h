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
#include "SubChannelMesh.h"

class TriSubChannelMesh;

/**
 * This class calculates the flow area of the triangular, edge, and corner subchannels for a
 * MARVEL-type microreactor core.
 */
class MarvelTriFlowAreaIC : public TriSubChannelBaseIC
{
public:
  MarvelTriFlowAreaIC(const InputParameters & params);
  Real value(const Point & p) override;

public:
  static InputParameters validParams();

protected:
  const SubChannelMesh & _subchannel_mesh;
};
