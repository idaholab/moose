//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeometricalComponent.h"
#include "FlowModel.h"
#include "FlowConnection.h"
#include "GravityInterface.h"

/**
 * Base class for geometrical components that have fluid flow
 */
class GeometricalFlowComponent : public GeometricalComponent, public GravityInterface
{
public:
  GeometricalFlowComponent(const InputParameters & parameters);

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

  /**
   * Gets the gravity angle for this component
   *
   * @return gravity angle for this component
   */
  virtual const Real & getGravityAngle() const { return _gravity_angle; }

  /**
   * Gets the vector of connections of an end type for this component
   *
   * @param[in] end_type   end type for the connections to get
   */
  virtual const std::vector<Connection> & getConnections(FlowConnection::EEndType end_type) const;

  /**
   * Gets the name of the fluid properties user object for this component
   */
  const UserObjectName & getFluidPropertiesName() const { return _fp_name; }

  /**
   * Gets the flow model ID
   */
  virtual const THM::FlowModelID & getFlowModelID() const = 0;

  /**
   * Gets the numerical flux user object name
   */
  const UserObjectName & getNumericalFluxUserObjectName() const { return _numerical_flux_name; }

  /**
   * Gets the RDG interfacial variables user object name
   */
  const UserObjectName & getRDGInterfacialVariablesUserObjectName() const
  {
    return _rdg_int_var_uo_name;
  }

  /**
   * Gets the slope reconstruction option used
   */
  const MooseEnum & getSlopeReconstruction() const { return _rdg_slope_reconstruction; }

protected:
  virtual bool usingSecondOrderMesh() const override;

  /// Angle between orientation vector and gravity vector, in degrees
  const Real _gravity_angle;

  /// Map of end type to a list of connections
  std::map<FlowConnection::EEndType, std::vector<Connection>> _connections;

  /// Name of fluid properties user object
  const UserObjectName & _fp_name;

  /// Numerical flux user object name
  const UserObjectName _numerical_flux_name;

  /// rDG interfacial variables user object name
  const UserObjectName _rdg_int_var_uo_name;

  /// Slope reconstruction type for rDG
  const MooseEnum _rdg_slope_reconstruction;

public:
  static InputParameters validParams();
};
