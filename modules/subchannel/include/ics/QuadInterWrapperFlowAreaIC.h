#pragma once

#include "QuadSubChannelBaseIC.h"
#include "QuadSubChannelMesh.h"
#include "QuadInterWrapperBaseIC.h"

/**
 * This class calculates the cross-sectional flow area of the quadrilateral subchannel
 */
class QuadInterWrapperFlowAreaIC : public QuadInterWrapperBaseIC
{
public:
  QuadInterWrapperFlowAreaIC(const InputParameters & params);
  Real value(const Point & p);

public:
  static InputParameters validParams();
};
