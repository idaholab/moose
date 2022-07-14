#pragma once
#include "InitialCondition.h"
#include "QuadSubChannelBaseIC.h"

class QuadInterWrapperMesh;

/**
 * An abstract class for ICs for quadrilateral subchannels
 */
class QuadInterWrapperBaseIC : public InitialCondition
{
public:
  QuadInterWrapperBaseIC(const InputParameters & params);

protected:
  /**
   * Check that `mesh` is QuadSubChannelMesh and if not, report an error.
   */
  QuadInterWrapperMesh & getMesh(MooseMesh & mesh);

  QuadInterWrapperMesh & _mesh;

public:
  static InputParameters validParams();
};
