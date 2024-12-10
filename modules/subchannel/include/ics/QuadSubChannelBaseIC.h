//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InitialCondition.h"

class QuadSubChannelMesh;

/**
 * An abstract class for ICs for quadrilateral subchannels
 */
class QuadSubChannelBaseIC : public InitialCondition
{
public:
  QuadSubChannelBaseIC(const InputParameters & params);

protected:
  /**
   * Check that `mesh` is QuadSubChannelMesh and if not, report an error.
   */
  QuadSubChannelMesh & getMesh(MooseMesh & mesh);

  QuadSubChannelMesh & _mesh;

public:
  static InputParameters validParams();
};
