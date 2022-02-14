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
  /**
   * Check that `mesh` is QuadSubChannelMesh and if not, report an error.
   */
  QuadSubChannelMesh & getMesh(MooseMesh & mesh);

  QuadSubChannelMesh & _mesh;

public:
  static InputParameters validParams();
};
