//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InterWrapperSolutionTransferBase.h"

/**
 * Transfers inter-wrapper solution from computational mesh onto visualization mesh
 */
class InterWrapperSolutionTransfer : public InterWrapperSolutionTransferBase
{
public:
  InterWrapperSolutionTransfer(const InputParameters & parameters);

protected:
  virtual Node * getFromNode(const InterWrapperMesh & from_mesh, const Point & src_node);

public:
  static InputParameters validParams();
};
