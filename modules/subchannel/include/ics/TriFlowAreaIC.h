#pragma once

#include "TriSubChannelBaseIC.h"
#include "SubChannelMesh.h"

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

public:
  static InputParameters validParams();

protected:
  SubChannelMesh & _subchannel_mesh;
};
