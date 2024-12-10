//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SCMSolutionTransferBase.h"

/**
 * Transfers subchannel solution from computational mesh onto visualization mesh
 */
class SCMPinSolutionTransfer : public SCMSolutionTransferBase
{
public:
  SCMPinSolutionTransfer(const InputParameters & parameters);

protected:
  virtual Node * getFromNode(const SubChannelMesh & from_mesh, const Point & src_node);

public:
  static InputParameters validParams();
};
