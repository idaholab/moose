#pragma once

#include "SubChannelBaseIC.h"

/**
 * Sets the wetted perimeter of the subchannel
 */
class WettedPerimIC : public SubChannelBaseIC
{
public:
  WettedPerimIC(const InputParameters & params);
  Real value(const Point & p) override;

public:
  static InputParameters validParams();
};
