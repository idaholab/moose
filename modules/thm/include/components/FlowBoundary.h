#pragma once

#include "FlowConnection.h"

/**
 * Base class for boundary components connected to flow channels
 */
class FlowBoundary : public FlowConnection
{
public:
  FlowBoundary(const InputParameters & params);

protected:
  virtual void init() override;
  virtual void setupMesh() override;

  /**
   * Creates the boundary condition objects for 1-phase flow
   */
  void addWeakBC3Eqn();

  /**
   * Creates the boundary condition objects for 2-phase flow
   */
  void addWeakBC7Eqn();

  /**
   * Creates the boundary condition objects for 2-phase flow with NCG
   */
  void addWeakBC7EqnNCG();

  /// The name of the connect flow channel
  std::string _connected_flow_channel_name;
  /// The end type of the connected flow channel
  EEndType _connected_flow_channel_end_type;

  /// Node ID
  dof_id_type _node;
  /// out norm on this boundary
  Real _normal;
  /// Name of the boundary this component attaches to
  BoundaryName _input;

  /// Numerical flux user object name
  UserObjectName _numerical_flux_name;
  /// rDG interfacial variables user object name
  UserObjectName _rdg_int_var_uo_name;
  /// Name of boundary user object name
  const UserObjectName _boundary_uo_name;

public:
  static InputParameters validParams();
};
