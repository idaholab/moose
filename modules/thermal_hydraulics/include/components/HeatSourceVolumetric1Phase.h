#pragma once

#include "Component.h"

/**
 * Volumetric heat source applied on a 1-phase flow channel
 */
class HeatSourceVolumetric1Phase : public Component
{
public:
  HeatSourceVolumetric1Phase(const InputParameters & parameters);

  virtual void check() const override;
  virtual void addMooseObjects() override;

public:
  static InputParameters validParams();
};
