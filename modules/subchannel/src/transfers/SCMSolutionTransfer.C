//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SCMSolutionTransfer.h"
#include "SubChannelMesh.h"

registerMooseObject("SubChannelApp", SCMSolutionTransfer);

InputParameters
SCMSolutionTransfer::validParams()
{
  InputParameters params = SCMSolutionTransferBase::validParams();
  params.addClassDescription(
      "Transfers subchannel solution from computational mesh onto visualization mesh");
  return params;
}

SCMSolutionTransfer::SCMSolutionTransfer(const InputParameters & parameters)
  : SCMSolutionTransferBase(parameters)
{
}

Node *
SCMSolutionTransfer::getFromNode(const SubChannelMesh & from_mesh, const Point & src_node)
{
  unsigned int sch_idx = from_mesh.channelIndex(src_node);
  unsigned iz = from_mesh.getZIndex(src_node);
  return from_mesh.getChannelNode(sch_idx, iz);
}
