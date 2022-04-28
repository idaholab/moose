#pragma once

#include "MultiAppInterWrapperSolutionTransferBase.h"

/**
 * Transfers inter-wrapper solution from computational mesh onto visualization mesh
 */
class MultiAppInterWrapperSolutionTransfer : public MultiAppInterWrapperSolutionTransferBase
{
public:
  MultiAppInterWrapperSolutionTransfer(const InputParameters & parameters);

protected:
  virtual Node * getFromNode(const InterWrapperMesh & from_mesh, const Point & src_node);

public:
  static InputParameters validParams();
};
