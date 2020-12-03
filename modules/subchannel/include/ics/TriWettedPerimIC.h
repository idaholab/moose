#pragma once

#include "TriSubChannelBaseIC.h"

/**
 * Sets the wetted perimeter of the triangular, edge, and corner subchannels for hexagonal fuel
 * assemblies
 */
class TriWettedPerimIC : public TriSubChannelBaseIC
{
public:
  TriWettedPerimIC(const InputParameters & params);
  Real value(const Point & p) override;

protected:
  TriSubChannelMesh & _mesh;

public:
  static InputParameters validParams();
};
