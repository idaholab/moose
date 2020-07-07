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
  /**
   * Find the (row, column) indices of the subchannel containing a given point.
   */
  std::pair<unsigned int, unsigned int> index_point(const Point & p) const;
  SubChannelMesh & _mesh;

public:
  static InputParameters validParams();
};
