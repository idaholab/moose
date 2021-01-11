#pragma once

#include "InitialCondition.h"

class TriSubChannelMesh;

/**
 * An abstract class for ICs for hexagonal fuel assemblies
 */
class TriSubChannelBaseIC : public InitialCondition
{
public:
  TriSubChannelBaseIC(const InputParameters & params);

protected:
  TriSubChannelMesh & _mesh;

public:
  static InputParameters validParams();
};
