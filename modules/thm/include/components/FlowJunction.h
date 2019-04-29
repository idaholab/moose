#pragma once

#include "FlowConnection.h"

class FlowJunction;

template <>
InputParameters validParams<FlowJunction>();

/**
 * Base class for flow junctions
 */
class FlowJunction : public FlowConnection
{
public:
  FlowJunction(const InputParameters & params);

protected:
  virtual void setupMesh() override;
};
