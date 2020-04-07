#pragma once

#include "FlowConnection.h"

/**
 * Base class for flow junctions
 */
class FlowJunction : public FlowConnection
{
public:
  FlowJunction(const InputParameters & params);

protected:
  virtual void setupMesh() override;

public:
  static InputParameters validParams();
};
