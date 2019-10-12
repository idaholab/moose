#pragma once

#include "Component.h"

class HeatGeneration;

template <>
InputParameters validParams<HeatGeneration>();

/**
 * Adds heat generation to a heat structure
 *
 * DEPRECATED
 */
class HeatGeneration : public Component
{
public:
  HeatGeneration(const InputParameters & parameters);
};
