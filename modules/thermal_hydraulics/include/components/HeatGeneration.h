#pragma once

#include "Component.h"

/**
 * Adds heat generation to a heat structure
 *
 * DEPRECATED
 */
class HeatGeneration : public Component
{
public:
  HeatGeneration(const InputParameters & parameters);

public:
  static InputParameters validParams();
};
