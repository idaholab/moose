/********************************************************************/
/*                   DO NOT MODIFY THIS HEADER                      */
/*          Subchannel: Thermal Hydraulics Reactor Analysis         */
/*                                                                  */
/*              (c) 2022 Battelle Energy Alliance, LLC              */
/*                      ALL RIGHTS RESERVED                         */
/*                                                                  */
/*             Prepared by Battelle Energy Alliance, LLC            */
/*               Under Contract No. DE-AC07-05ID14517               */
/*               With the U. S. Department of Energy                */
/*                                                                  */
/*               See COPYRIGHT for full restrictions                */
/********************************************************************/

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
