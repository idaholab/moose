#ifndef PIPEBASE_H
#define PIPEBASE_H

#include "libmesh/point.h"
#include "libmesh/vector_value.h"

#include "RELAP7App.h"
#include "GeometricalComponent.h"
#include "FlowModel.h"

class PipeBase;

template<>
InputParameters validParams<PipeBase>();

/**
 * A base class for pipe components
 *
 * A pipe is defined by its position, direction, length and area.
 * Mesh: mesh is generated in such a way, that the pipe starts at the origin (0, 0, 0) and is aligned with x-axis. Its
 * subdivided into _n_elems elements (of type EDGE2).
 */
class PipeBase : public GeometricalComponent,
    public FlowModel
{
public:
  PipeBase(const std::string & name, InputParameters params);
  virtual ~PipeBase();

  virtual const std::string & getType() { return _type; }

  // Pipe specific interface ----
  virtual Real getLength() = 0;
  virtual UserObjectName getFluidPropertiesName() const = 0;

public:
  static const std::string _type;
};

#endif /* PIPEBASE_H */
