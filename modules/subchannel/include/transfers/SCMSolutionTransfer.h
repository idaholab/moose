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

#pragma once

#include "MultiAppDetailedSolutionTransferBase.h"

/**
 * Transfers subchannel solution from computational mesh onto visualization mesh
 */
class SCMSolutionTransfer : public MultiAppDetailedSolutionTransferBase
{
public:
  SCMSolutionTransfer(const InputParameters & parameters);

protected:
  Node * getFromNode(const SubChannelMesh & from_mesh, const Point & src_node) override;

public:
  static InputParameters validParams();
};
