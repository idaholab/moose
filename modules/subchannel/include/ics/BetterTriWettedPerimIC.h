#pragma once

#include "BetterTriSubChannelBaseIC.h"

/**
 * Sets the wetted perimeter of the triangular, edge, and corner subchannels for hexagonal fuel
 * assemblies
 */
class BetterTriWettedPerimIC : public BetterTriSubChannelBaseIC
{
public:
  BetterTriWettedPerimIC(const InputParameters & params);
  Real value(const Point & p) override;

public:
  static InputParameters validParams();
};
