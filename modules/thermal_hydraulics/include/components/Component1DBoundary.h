//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Component1DConnection.h"

/**
 * Base class for boundary components connected to 1D components
 */
class Component1DBoundary : public Component1DConnection
{
public:
  Component1DBoundary(const InputParameters & params);

protected:
  virtual void setupMesh() override;
  virtual void check() const override;

  /// Name of the connected component
  std::string _connected_component_name;
  /// End type of the connected component
  EEndType _connected_component_end_type;

  /// Node ID
  dof_id_type _node;
  /// Outward normal on this boundary
  Real _normal;
  /// Name of the boundary this component attaches to
  BoundaryName _input;

public:
  static InputParameters validParams();
};
