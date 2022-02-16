#pragma once

#include "MultiAppDetailedSolutionTransferBase.h"

/**
 * Transfers subchannel solution from computational mesh onto visualization mesh
 */
class MultiAppDetailedPinSolutionTransfer : public MultiAppDetailedSolutionTransferBase
{
public:
  MultiAppDetailedPinSolutionTransfer(const InputParameters & parameters);

protected:
  virtual Node * getFromNode(const SubChannelMesh & from_mesh, const Point & src_node);

public:
  static InputParameters validParams();
};
