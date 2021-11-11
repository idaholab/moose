#pragma once

#include "BetterTriSubChannelBaseIC.h"

class BetterTriSubChannelMesh;

/**
 * This class calculates the area of the triangular, edge, and corner subchannels for hexagonal fuel
 * assemblies
 */
class BetterTriFlowAreaIC : public BetterTriSubChannelBaseIC
{
public:
  BetterTriFlowAreaIC(const InputParameters & params);
  Real value(const Point & p) override;

public:
  static InputParameters validParams();
};
