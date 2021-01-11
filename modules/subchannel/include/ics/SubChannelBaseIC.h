#pragma once

#include "InitialCondition.h"

class SubChannelMesh;

/**
 * An abstract class for ICs
 */
class SubChannelBaseIC : public InitialCondition
{
public:
  SubChannelBaseIC(const InputParameters & params);

protected:
  SubChannelMesh & _mesh;

public:
  static InputParameters validParams();
};
