//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MaxVarNDofsPerNode.h"
#include "NonlinearSystem.h"
#include "Problem.h"
#include "Damper.h"

// libmesh includes
#include "libmesh/threads.h"

MaxVarNDofsPerNode::MaxVarNDofsPerNode(FEProblemBase & feproblem, NonlinearSystemBase & sys)
  : ThreadedNodeLoop<ConstNodeRange, ConstNodeRange::const_iterator>(feproblem),
    _system(sys),
    _max(0),
    _dof_map(_system.dofMap())
{
}

// Splitting Constructor
MaxVarNDofsPerNode::MaxVarNDofsPerNode(MaxVarNDofsPerNode & x, Threads::split split)
  : ThreadedNodeLoop<ConstNodeRange, ConstNodeRange::const_iterator>(x, split),
    _system(x._system),
    _max(0),
    _dof_map(x._dof_map)
{
}

MaxVarNDofsPerNode::~MaxVarNDofsPerNode() {}

void
MaxVarNDofsPerNode::onNode(ConstNodeRange::const_iterator & node_it)
{
  for (unsigned int var = 0; var < _system.nVariables(); var++)
  {
    _dof_map.dof_indices(*node_it, _dof_indices, var);

    _max = std::max(_max, _dof_indices.size());
  }
}

void
MaxVarNDofsPerNode::join(const MaxVarNDofsPerNode & y)
{
  _max = std::max(_max, y._max);
}
