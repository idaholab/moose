//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FlowChannel1Phase.h"

/**
 * Bent pipe for 1-phase flow
 */
class ElbowPipe1Phase : public FlowChannel1Phase
{
public:
  ElbowPipe1Phase(const InputParameters & params);

protected:
  virtual void buildMeshNodes() override;

  /// Radius of the pipe [m]
  Real _radius;
  /// Start angle [degrees]
  Real _start_angle;
  /// End angle [degrees]
  Real _end_angle;
  /// central angle
  Real _central_angle;

public:
  static InputParameters validParams();
};
