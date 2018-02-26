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
  /// Name of the input
  std::string _input;

private:
  virtual std::string createBoundaryName(const std::string & comp_name) const override;
};

#endif /* FLOWBOUNDARY_H */
