#pragma once

#include "InitialCondition.h"

class BetterQuadSubChannelMesh;

/**
 * An abstract class for ICs for quadrilateral subchannels
 */
class BetterQuadSubChannelBaseIC : public InitialCondition
{
public:
  BetterQuadSubChannelBaseIC(const InputParameters & params);

protected:
  BetterQuadSubChannelMesh & _mesh;

public:
  static InputParameters validParams();
};
