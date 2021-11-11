#pragma once

#include "InitialCondition.h"

class BetterTriSubChannelMesh;

/**
 * An abstract class for ICs for hexagonal fuel assemblies
 */
class BetterTriSubChannelBaseIC : public InitialCondition
{
public:
  BetterTriSubChannelBaseIC(const InputParameters & params);

protected:
  BetterTriSubChannelMesh & _mesh;

public:
  static InputParameters validParams();
};
