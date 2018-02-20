#ifndef PIPEBASE_H
#define PIPEBASE_H

#include "libmesh/point.h"
#include "libmesh/vector_value.h"

#include "RELAP7App.h"
#include "GeometricalComponent.h"

class PipeBase;

template <>
InputParameters validParams<PipeBase>();

/**
 * A base class for pipe components
 *
 * A pipe is defined by its position, direction, length and area.
 * Mesh: mesh is generated in such a way, that the pipe starts at the origin (0, 0, 0) and is
 * aligned with x-axis. Its
 * subdivided into _n_elems elements (of type EDGE2).
 */
class PipeBase : public GeometricalComponent
{
public:
  PipeBase(const InputParameters & params);

  // Pipe specific interface ----
  virtual UserObjectName getFluidPropertiesName() const { return _fp_name; }
  virtual std::shared_ptr<const FlowModel> getFlowModel() const;
  virtual const RELAP7::FlowModelID & getFlowModelID() const;
  virtual unsigned int getSubdomainID() const = 0;
  virtual bool isHorizontal() const = 0;
  virtual Real getInclinedAngle() const = 0;

protected:
  virtual void init() override;

  /// The name of the user object that defines fluid properties
  const UserObjectName & _fp_name;
  /// The flow model used by this pipe
  std::shared_ptr<FlowModel> _flow_model;
  /// The flow model type used by this pipe
  RELAP7::FlowModelID _model_id;
};

#endif /* PIPEBASE_H */
