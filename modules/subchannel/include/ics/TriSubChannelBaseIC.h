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
   * Find the channel indice of the subchannel containing a given point.
   */
  unsigned int index_point(const Point & p) const;

  TriSubChannelMesh & _mesh;

public:
  static InputParameters validParams();
};
