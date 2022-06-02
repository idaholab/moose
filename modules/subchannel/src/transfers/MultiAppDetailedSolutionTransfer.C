#include "MultiAppDetailedSolutionTransfer.h"
#include "SubChannelMesh.h"

registerMooseObject("SubChannelApp", MultiAppDetailedSolutionTransfer);

InputParameters
MultiAppDetailedSolutionTransfer::validParams()
{
  InputParameters params = MultiAppDetailedSolutionTransferBase::validParams();
  params.addClassDescription(
      "Transfers subchannel solution from computational mesh onto visualization mesh");
  return params;
}

MultiAppDetailedSolutionTransfer::MultiAppDetailedSolutionTransfer(
    const InputParameters & parameters)
  : MultiAppDetailedSolutionTransferBase(parameters)
{
}

Node *
MultiAppDetailedSolutionTransfer::getFromNode(const SubChannelMesh & from_mesh,
                                              const Point & src_node)
{
  unsigned int sch_idx = from_mesh.channelIndex(src_node);
  unsigned iz = from_mesh.getZIndex(src_node);
  std::cout << "Subchannel: " << sch_idx << "  -  " << "Layer: " << iz << std::endl;
  return from_mesh.getChannelNode(sch_idx, iz);
}
