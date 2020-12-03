#pragma once

#include "TriSubChannelBaseIC.h"

class TriSubChannelMesh;

/**
 * This class calculates the area of the triangular, edge, and corner subchannels for hexagonal fuel
 * assemblies
 */
class TriFlowAreaIC : public TriSubChannelBaseIC
{
public:
  TriFlowAreaIC(const InputParameters & params);
  Real value(const Point & p) override;

protected:
  TriSubChannelMesh & _mesh;

public:
  static InputParameters validParams();
};
