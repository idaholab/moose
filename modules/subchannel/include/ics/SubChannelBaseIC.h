#pragma once

#include "InitialCondition.h"

class QuadSubChannelMesh;

/**
 * An abstract class for ICs
 */
class SubChannelBaseIC : public InitialCondition
{
public:
  SubChannelBaseIC(const InputParameters & params);

protected:
  QuadSubChannelMesh & _mesh;

public:
  static InputParameters validParams();
};
