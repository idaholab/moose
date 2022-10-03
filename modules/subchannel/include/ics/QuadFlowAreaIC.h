#pragma once

#include "QuadSubChannelBaseIC.h"
#include "QuadSubChannelMesh.h"

/**
 * This class calculates the cross-sectional flow area of the quadrilateral subchannel
 */
class QuadFlowAreaIC : public QuadSubChannelBaseIC
{
public:
  QuadFlowAreaIC(const InputParameters & params);
  Real value(const Point & p) override;

public:
  static InputParameters validParams();

protected:
  SubChannelMesh & _subchannel_mesh;
};
