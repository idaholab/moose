//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Component1D.h"

InputParameters
Component1D::validParams()
{
  InputParameters params = GeometricalComponent::validParams();
  return params;
}

Component1D::Component1D(const InputParameters & parameters) : GeometricalComponent(parameters) {}

bool
Component1D::usingSecondOrderMesh() const
{
  return false;
}

const std::vector<Component1D::Connection> &
Component1D::getConnections(Component1DConnection::EEndType end_type) const
{
  checkSetupStatus(MESH_PREPARED);

  std::map<Component1DConnection::EEndType, std::vector<Connection>>::const_iterator it =
      _connections.find(end_type);
  if (it != _connections.end())
    return it->second;
  else
    mooseError(name(), ": Invalid end type (", end_type, ").");
}
