//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FlowJunction.h"

/**
 * Base class for volumetric junction components
 */
class VolumeJunctionBase : public FlowJunction
{
public:
  VolumeJunctionBase(const InputParameters & params);

protected:
  virtual void setupMesh() override;

  /// Volume of the junction
  const Real _volume;

  /// Spatial position of center of the junction
  const Point & _position;

  /// Element IDs of connected flow channels
  std::vector<dof_id_type> _connected_elems;

public:
  static InputParameters validParams();
};
