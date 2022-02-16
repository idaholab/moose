#pragma once

#include "MultiAppTransfer.h"

class SubChannelMesh;

/**
 * Base class for transfering solutions from computational mesh onto visualization mesh
 */
class MultiAppDetailedSolutionTransferBase : public MultiAppTransfer
{
public:
  MultiAppDetailedSolutionTransferBase(const InputParameters & parameters);

  virtual void execute() override;

protected:
  /**
   * Do the transfer into the sub-app
   */
  void transferToMultiApps();

  /**
   * Transfer variables into the sub-app
   *
   * @param app_idx Multi-app index
   */
  void transferVarsToApp(unsigned int app_idx);

  void transferNodalVars(unsigned int app_idx);

  /**
   * Find node on computational mesh given the visualization point
   *
   * @return Node from the computational mesh
   * @param from_mesh Computational mesh
   * @param src_node Node from the visualization
   */
  virtual Node * getFromNode(const SubChannelMesh & from_mesh, const Point & src_node) = 0;

  /// Variable names to transfer
  const std::vector<AuxVariableName> & _var_names;

public:
  static InputParameters validParams();
};
