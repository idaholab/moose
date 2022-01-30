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

  /// Name of junction user object name, if any
  const std::string _junction_uo_name;

public:
  static InputParameters validParams();
};
