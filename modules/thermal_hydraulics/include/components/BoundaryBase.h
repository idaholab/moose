#pragma once

#include "Component.h"

/**
 * Base class for components of a boundary type
 *
 */
class BoundaryBase : public Component
{
public:
  BoundaryBase(const InputParameters & params);

public:
  static InputParameters validParams();
};
