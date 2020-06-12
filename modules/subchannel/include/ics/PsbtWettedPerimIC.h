#pragma once

#include "PsbtIC.h"

/**
 * Sets the linear heat rate for the PSBT 01-6232 fluid temperature benchmark.
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
