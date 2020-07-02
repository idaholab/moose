#pragma once

#include "IC.h"
#include "SolutionHandle.h"
#include "SubChannelMesh.h"

/**
 * This class calculates the area of the subchannel
 */
class FlowAreaIC : public IC
{
public:
  FlowAreaIC(const InputParameters & params);

  Real value(const Point & p) override;

protected:
  SubChannelMesh & _mesh;

public:
  static InputParameters validParams();
};
