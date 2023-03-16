//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneratedMeshComponent.h"
#include "Component1DConnection.h"

/**
 * Base class for 1D components
 */
class Component1D : public GeneratedMeshComponent
{
public:
  Component1D(const InputParameters & parameters);

  /// Structure for storing connection data
  struct Connection
  {
    /// Physical position of the connecting point
    Point _position;
    /// Boundary node of connection (used by other components for connecting)
    Node * _node;
    /// Boundary id of this connection
    unsigned int _boundary_id;
    /// Outward norm (either 1 or -1) on boundaries
    Real _normal;

    Connection(const Point & pt, Node * node, unsigned int bc_id, Real normal)
      : _position(pt), _node(node), _boundary_id(bc_id), _normal(normal)
    {
    }
  };

  virtual void buildMesh() override;

  /**
   * Gets the 1D component nodeset ID
   */
  unsigned int getNodesetID() const;

  /**
   * Gets the 1D component nodeset name
   */
  const BoundaryName & getNodesetName() const;

  /**
   * Gets the vector of connections of an end type for this component
   *
   * @param[in] end_type   end type for the connections to get
   */
  virtual const std::vector<Connection> &
  getConnections(Component1DConnection::EEndType end_type) const;

protected:
  virtual bool usingSecondOrderMesh() const override;

  /// Map of end type to a list of connections
  std::map<Component1DConnection::EEndType, std::vector<Connection>> _connections;

private:
  virtual void buildMeshNodes();

  /// Nodeset ID for all 1D component nodes
  BoundaryID _nodeset_id;
  /// Nodeset name for all 1D component nodes
  BoundaryName _nodeset_name;

public:
  static InputParameters validParams();
};
