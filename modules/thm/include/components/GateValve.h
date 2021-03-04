#pragma once

#include "FlowJunction.h"

/**
 * Gate valve component
 *
 * Deprecated
 */
class GateValve : public FlowJunction
{
public:
  GateValve(const InputParameters & params);

public:
  static InputParameters validParams();
};
