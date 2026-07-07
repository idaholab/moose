//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MultiAppTransfer.h"

class SubChannelMesh;
class FEProblemBase;

/**
 * Transfers subchannel and pin solutions from a SubChannel mesh onto a visualization mesh
 */
class SCMSolutionTransfer : public MultiAppTransfer
{
public:
  SCMSolutionTransfer(const InputParameters & parameters);

  virtual void execute() override;
  void initialSetup() override;

protected:
  void transferToMultiApps();
  void transferVarsToApp(unsigned int app_idx);
  void transferNodalVars(unsigned int app_idx);
  void validateVariableLocations(const SubChannelMesh & from_mesh, FEProblemBase & to_problem);
  Node * getFromNode(const SubChannelMesh & from_mesh, const Point & src_node);

  /// Variable names to transfer
  const std::vector<AuxVariableName> & _var_names;

  /// Whether pin fields should be transferred instead of subchannel fields
  const bool _pin_transfer;

public:
  static InputParameters validParams();
};
