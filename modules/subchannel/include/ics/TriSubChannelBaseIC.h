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
  /**
   * Check that `mesh` is TriSubChannelMesh and if not, report an error.
   */
  TriSubChannelMesh & getMesh(MooseMesh & mesh);

  TriSubChannelMesh & _mesh;

public:
  static InputParameters validParams();
};
