#pragma once

#include "Component.h"

/**
 * Base class for creating component that connect other components together (e.g. a flow channel and
 * a heat structure)
 */
class ConnectorBase : public Component
{
public:
  ConnectorBase(const InputParameters & params);

public:
  static InputParameters validParams();
};
