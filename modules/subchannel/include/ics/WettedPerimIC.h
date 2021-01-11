#pragma once

#include "QuadSubChannelBaseIC.h"

/**
 * Sets the wetted perimeter of the subchannel
 */
class WettedPerimIC : public QuadSubChannelBaseIC
{
public:
  WettedPerimIC(const InputParameters & params);
  Real value(const Point & p) override;

public:
  static InputParameters validParams();
};
