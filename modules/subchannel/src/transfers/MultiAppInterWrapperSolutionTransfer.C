#include "MultiAppInterWrapperSolutionTransfer.h"
#include "InterWrapperMesh.h"

registerMooseObject("SubChannelApp", MultiAppInterWrapperSolutionTransfer);

InputParameters
MultiAppInterWrapperSolutionTransfer::validParams()
{
  InputParameters params = MultiAppInterWrapperSolutionTransferBase::validParams();
  params.addClassDescription(
      "Transfers Inter-Wrapper solution from computational mesh onto visualization mesh");
  return params;
}

MultiAppInterWrapperSolutionTransfer::MultiAppInterWrapperSolutionTransfer(
    const InputParameters & parameters)
  : MultiAppInterWrapperSolutionTransferBase(parameters)
{
}

Node *
MultiAppInterWrapperSolutionTransfer::getFromNode(const InterWrapperMesh & from_mesh,
                                              const Point & src_node)
{
  unsigned int sch_idx = from_mesh.channelIndex(src_node);
  unsigned iz = from_mesh.getZIndex(src_node);
  return from_mesh.getChannelNode(sch_idx, iz);
}
