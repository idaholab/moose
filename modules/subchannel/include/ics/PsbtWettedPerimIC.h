#pragma once

#include "PsbtIC.h"

/**
 * Sets the wetted perimeter of the subchannel
 */
class PsbtWettedPerimIC : public PsbtIC
{
public:
  PsbtWettedPerimIC(const InputParameters & params);

  Real value(const Point & p) override;

protected:
  SubChannelMesh * _mesh;

public:
  static InputParameters validParams();
};
