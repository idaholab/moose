#pragma once

#include "InitialCondition.h"
#include "TriSubChannelBaseIC.h"

class TriInterWrapperMesh;

/**
 * An abstract class for ICs for hexagonal fuel assemblies
 */
class TriInterWrapperBaseIC : public InitialCondition
{
public:
  TriInterWrapperBaseIC(const InputParameters & params);

protected:
  /**
   * Check that `mesh` is TriInterWrapperMesh and if not, report an error.
   */
  TriInterWrapperMesh & getMesh(MooseMesh & mesh);

  TriInterWrapperMesh & _mesh;

public:
  static InputParameters validParams();
};
