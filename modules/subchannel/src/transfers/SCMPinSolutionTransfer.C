//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SCMPinSolutionTransfer.h"
#include "SubChannelMesh.h"

registerMooseObject("SubChannelApp", SCMPinSolutionTransfer);

InputParameters
SCMPinSolutionTransfer::validParams()
{
  InputParameters params = SCMSolutionTransferBase::validParams();
  params.addClassDescription(
      "Transfers subchannel solution from computational mesh onto visualization mesh");
  return params;
}

SCMPinSolutionTransfer::SCMPinSolutionTransfer(const InputParameters & parameters)
  : SCMSolutionTransferBase(parameters)
{
}

Node *
SCMPinSolutionTransfer::getFromNode(const SubChannelMesh & from_mesh, const Point & src_node)
{
  unsigned int pin_idx = from_mesh.pinIndex(src_node);
  unsigned iz = from_mesh.getZIndex(src_node);
  return from_mesh.getPinNode(pin_idx, iz);
}
