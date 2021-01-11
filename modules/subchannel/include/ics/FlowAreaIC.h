#pragma once

#include "QuadSubChannelBaseIC.h"
#include "QuadSubChannelMesh.h"

/**
 * This class calculates the area of the subchannel
 */
class FlowAreaIC : public QuadSubChannelBaseIC
{
public:
  FlowAreaIC(const InputParameters & params);
  Real value(const Point & p) override;

public:
  static InputParameters validParams();
};
