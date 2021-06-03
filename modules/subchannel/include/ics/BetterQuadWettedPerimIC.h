#pragma once

#include "BetterQuadSubChannelBaseIC.h"

/**
 * Sets the wetted perimeter of the quadrilater subchannel
 */
class BetterQuadWettedPerimIC : public BetterQuadSubChannelBaseIC
{
public:
  BetterQuadWettedPerimIC(const InputParameters & params);
  Real value(const Point & p) override;

public:
  static InputParameters validParams();
};
