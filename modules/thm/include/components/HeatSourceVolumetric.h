#pragma once

#include "Component.h"

class HeatSourceVolumetric;

template <>
InputParameters validParams<HeatSourceVolumetric>();

/**
 * Volumetric heat source applied on a flow channel
 */
class HeatSourceVolumetric : public Component
{
public:
  HeatSourceVolumetric(const InputParameters & parameters);

  virtual void check() const override;
  virtual void addMooseObjects() override;
};
