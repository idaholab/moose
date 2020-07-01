#pragma once

#include "IC.h"

/**
 * Sets the wetted perimeter of the subchannel
 */
class WettedPerimIC : public IC
{
public:
  WettedPerimIC(const InputParameters & params);

  Real value(const Point & p) override;

protected:
  SubChannelMesh * _mesh;

public:
  static InputParameters validParams();
};
