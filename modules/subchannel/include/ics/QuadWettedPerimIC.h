#pragma once

#include "QuadSubChannelBaseIC.h"

/**
 * Sets the wetted perimeter of the quadrilater subchannel
 */
class QuadWettedPerimIC : public QuadSubChannelBaseIC
{
public:
  QuadWettedPerimIC(const InputParameters & params);
  Real value(const Point & p) override;

public:
  static InputParameters validParams();
};
