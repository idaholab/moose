#ifndef FLOWBOUNDARY_H
#define FLOWBOUNDARY_H

#include "FlowConnection.h"

class FlowBoundary;

template <>
InputParameters validParams<FlowBoundary>();

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
   * Creates the boundary condition objects for 1-phase rDG
   *
   * @param[in] boundary_flux_name   name of the boundary flux user object
   */
  void createRDGBoundaryConditions1Phase(const std::string & boundary_flux_name);

  /// The name of the connect pipe
  std::string _connected_pipe_name;
  /// The end type of the connected pipe
  EEndType _connected_pipe_end_type;

  /// Flow model
  std::shared_ptr<const FlowModel> _flow_model;

  /// Node ID
  dof_id_type _node;
  /// out norm on this boundary
  Real _normal;
  /// Name of the boundary this component attaches to
  BoundaryName _input;

  /// rDG flux user object name
  UserObjectName _rdg_flux_name;
};

#endif /* FLOWBOUNDARY_H */
