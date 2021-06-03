#pragma once

#include "BetterQuadSubChannelBaseIC.h"
#include "BetterQuadSubChannelMesh.h"

/**
 * This class calculates the cross-sectional flow area of the quadrilateral subchannel
 */
class BetterQuadFlowAreaIC : public BetterQuadSubChannelBaseIC
{
public:
  BetterQuadFlowAreaIC(const InputParameters & params);
  Real value(const Point & p) override;

public:
  static InputParameters validParams();
};
