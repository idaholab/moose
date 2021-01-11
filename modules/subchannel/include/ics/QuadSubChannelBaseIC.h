#pragma once

#include "InitialCondition.h"

class QuadSubChannelMesh;

/**
 * An abstract class for ICs for quadrilateral subchannels
 */
class QuadSubChannelBaseIC : public InitialCondition
{
public:
  QuadSubChannelBaseIC(const InputParameters & params);

protected:
  QuadSubChannelMesh & _mesh;

public:
  static InputParameters validParams();
};
