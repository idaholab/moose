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

  virtual void init() override;

  virtual const std::string & getType() override { return _type; }

  // Pipe specific interface ----
  virtual UserObjectName getFluidPropertiesName() const;
  virtual std::shared_ptr<const FlowModel> getFlowModel() const { return _flow_model; }
  virtual const RELAP7::FlowModelID & getFlowModelID() const { return _model_id; }
  virtual unsigned int getSubdomainID() const = 0;
  virtual bool isHorizontal() const = 0;
  virtual Real getInclinedAngle() const = 0;

protected:
  /// The name of the user object that defines fluid properties
  UserObjectName _fp_name;
  /// The flow model used by this pipe
  std::shared_ptr<FlowModel> _flow_model;
  /// The flow model type used by this pipe
  RELAP7::FlowModelID _model_id;

public:
  static const std::string _type;
};

#endif /* PIPEBASE_H */
