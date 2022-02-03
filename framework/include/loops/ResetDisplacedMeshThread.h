//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "ThreadedNodeLoop.h"

#include "libmesh/node_range.h"
#include "libmesh/numeric_vector.h"

// Forward declarations
class DisplacedProblem;
class FEProblemBase;
class MooseMesh;

class ResetDisplacedMeshThread : public ThreadedNodeLoop<NodeRange, NodeRange::const_iterator>
{
public:
  ResetDisplacedMeshThread(FEProblemBase & fe_problem, DisplacedProblem & displaced_problem);

  ResetDisplacedMeshThread(ResetDisplacedMeshThread & x, Threads::split split);

  void onNode(NodeRange::const_iterator & nd);

  void join(const ResetDisplacedMeshThread & /*y*/);

protected:
  DisplacedProblem & _displaced_problem;
  MooseMesh & _ref_mesh;
};
