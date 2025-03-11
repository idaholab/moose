//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InterWrapperSolutionTransfer.h"
#include "InterWrapperMesh.h"

registerMooseObject("SubChannelApp", InterWrapperSolutionTransfer);

InputParameters
InterWrapperSolutionTransfer::validParams()
{
  InputParameters params = InterWrapperSolutionTransferBase::validParams();
  params.addClassDescription(
      "Transfers Inter-Wrapper solution from computational mesh onto visualization mesh");
  return params;
}

InterWrapperSolutionTransfer::InterWrapperSolutionTransfer(const InputParameters & parameters)
  : InterWrapperSolutionTransferBase(parameters)
{
}

Node *
InterWrapperSolutionTransfer::getFromNode(const InterWrapperMesh & from_mesh,
                                          const Point & src_node)
{
  unsigned int sch_idx = from_mesh.channelIndex(src_node);
  unsigned iz = from_mesh.getZIndex(src_node);
  return from_mesh.getChannelNode(sch_idx, iz);
}
