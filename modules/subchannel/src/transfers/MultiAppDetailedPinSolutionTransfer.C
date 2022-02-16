#include "MultiAppDetailedPinSolutionTransfer.h"
#include "SubChannelMesh.h"

registerMooseObject("SubChannelApp", MultiAppDetailedPinSolutionTransfer);

InputParameters
MultiAppDetailedPinSolutionTransfer::validParams()
{
  InputParameters params = MultiAppDetailedSolutionTransferBase::validParams();
  params.addClassDescription(
      "Transfers subchannel solution from computational mesh onto visualization mesh");
  return params;
}

MultiAppDetailedPinSolutionTransfer::MultiAppDetailedPinSolutionTransfer(
    const InputParameters & parameters)
  : MultiAppDetailedSolutionTransferBase(parameters)
{
}

Node *
MultiAppDetailedPinSolutionTransfer::getFromNode(const SubChannelMesh & from_mesh,
                                                 const Point & src_node)
{
  unsigned int pin_idx = from_mesh.pinIndex(src_node);
  unsigned iz = from_mesh.getZIndex(src_node);
  return from_mesh.getPinNode(pin_idx, iz);
}
