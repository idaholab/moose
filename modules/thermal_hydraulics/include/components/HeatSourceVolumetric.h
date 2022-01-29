#pragma once

#include "Component.h"

/**
 * Volumetric heat source applied on a flow channel
 *
 * Deprecated
 */
class HeatSourceVolumetric : public Component
{
public:
  HeatSourceVolumetric(const InputParameters & parameters);

public:
  static InputParameters validParams();
};
