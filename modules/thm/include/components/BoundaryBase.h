#pragma once

#include "Component.h"

class BoundaryBase;

template <>
InputParameters validParams<BoundaryBase>();

/**
 * Base class for components of a boundary type
 *
 */
class BoundaryBase : public Component
{
public:
  BoundaryBase(const InputParameters & params);
};
