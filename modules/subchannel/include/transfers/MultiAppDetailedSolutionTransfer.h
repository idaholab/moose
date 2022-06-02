#pragma once

#include "MultiAppDetailedSolutionTransferBase.h"

/**
 * Transfers subchannel solution from computational mesh onto visualization mesh
 */
class MultiAppDetailedSolutionTransfer : public MultiAppDetailedSolutionTransferBase
{
public:
  MultiAppDetailedSolutionTransfer(const InputParameters & parameters);

protected:
  Node * getFromNode(const SubChannelMesh & from_mesh, const Point & src_node) override;

public:
  static InputParameters validParams();
};
