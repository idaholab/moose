//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MultiAppTransfer.h"

class SubChannelMesh;

/**
 * Base class for transfering solutions from computational mesh onto visualization mesh
 */
class SCMSolutionTransferBase : public MultiAppTransfer
{
public:
  SCMSolutionTransferBase(const InputParameters & parameters);

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
